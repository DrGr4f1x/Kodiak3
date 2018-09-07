//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "Texture.h"

#include "BinaryReader.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"
#include "KTXTextureLoader.h"

#include "CommandContextVk.h"
#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


namespace
{

void GetSurfaceInfo(size_t width, size_t height, VkFormat format, size_t& numBytes, size_t& rowBytes, size_t& numRows)
{
	numBytes = 0;
	rowBytes = 0;
	numRows = 0;

	bool bc = false;
	bool packed = false;
	bool planar = false;
	size_t bpe = 0;
	switch (format)
	{
	case VK_FORMAT_BC1_RGB_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGB_UNORM_BLOCK:
	case VK_FORMAT_BC1_RGBA_SRGB_BLOCK:
	case VK_FORMAT_BC1_RGBA_UNORM_BLOCK:
	case VK_FORMAT_BC4_SNORM_BLOCK:
	case VK_FORMAT_BC4_UNORM_BLOCK:
		bc = true;
		bpe = 8;
		break;

	case VK_FORMAT_BC2_SRGB_BLOCK:
	case VK_FORMAT_BC2_UNORM_BLOCK:
	case VK_FORMAT_BC3_SRGB_BLOCK:
	case VK_FORMAT_BC3_UNORM_BLOCK:
	case VK_FORMAT_BC5_SNORM_BLOCK:
	case VK_FORMAT_BC5_UNORM_BLOCK:
	case VK_FORMAT_BC6H_SFLOAT_BLOCK:
	case VK_FORMAT_BC6H_UFLOAT_BLOCK:
	case VK_FORMAT_BC7_SRGB_BLOCK:
	case VK_FORMAT_BC7_UNORM_BLOCK:
		bc = true;
		bpe = 16;
		break;

	// TODO: Figure out VK equivalents of these formats
	/*case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_YUY2:
		packed = true;
		bpe = 4;
		break;

	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		packed = true;
		bpe = 8;
		break;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
		planar = true;
		bpe = 2;
		break;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		planar = true;
		bpe = 4;
		break;*/
	}

	if (bc)
	{
		size_t numBlocksWide = 0;
		if (width > 0)
		{
			numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
		}
		size_t numBlocksHigh = 0;
		if (height > 0)
		{
			numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
		}
		rowBytes = numBlocksWide * bpe;
		numRows = numBlocksHigh;
		numBytes = rowBytes * numBlocksHigh;
	}
	else if (packed)
	{
		rowBytes = ((width + 1) >> 1) * bpe;
		numRows = height;
		numBytes = rowBytes * height;
	}
	else if (planar)
	{
		rowBytes = ((width + 1) >> 1) * bpe;
		numBytes = (rowBytes * height) + ((rowBytes * height + 1) >> 1);
		numRows = height + ((height + 1) >> 1);
	}
	else
	{
		size_t bpp = BitsPerPixel(MapVulkanFormatToEngine(format));
		rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
		numRows = height;
		numBytes = rowBytes * height;
	}
}


bool QueryLinearTilingFeature(VkFormatProperties properties, VkFormatFeatureFlagBits flags)
{
	return (properties.linearTilingFeatures & flags) != 0;
}


bool QueryOptimalTilingFeature(VkFormatProperties properties, VkFormatFeatureFlagBits flags)
{
	return (properties.optimalTilingFeatures & flags) != 0;
}


bool QueryBufferFeature(VkFormatProperties properties, VkFormatFeatureFlagBits flags)
{
	return (properties.bufferFeatures & flags) != 0;
}

} // anonymous namespace


namespace Kodiak
{

struct TextureInitializer::PlatformData
{
	PlatformData(uint32_t numMips) : faceOffsets(numMips) {}

	std::unique_ptr<byte[]> data;
	std::vector<size_t>		faceOffsets;
	size_t					arraySliceSize{ 0 };
	size_t					totalSize{ 0 };
};

} // namespace Kodiak


TextureInitializer::~TextureInitializer()
{
	delete m_platformData;
}


void TextureInitializer::PlatformCreate()
{
	m_platformData = new PlatformData(m_numMips);

	size_t sizeBytes = 0;
	auto format = static_cast<VkFormat>(m_format);

	if (m_type == ResourceType::Texture3D)
	{
		uint32_t width = m_width;
		uint32_t height = m_height;
		uint32_t depth = m_depthOrArraySize;

		for (uint32_t j = 0; j < m_numMips; ++j)
		{
			size_t numBytes;
			size_t rowBytes;
			size_t numRows;
			GetSurfaceInfo(width, height, format, numBytes, rowBytes, numRows);
			numBytes *= depth;

			sizeBytes += numBytes;

			m_faceSize[j] = numBytes;

			m_platformData->faceOffsets[j] = (j == 0) ? 0 : m_platformData->faceOffsets[j - 1] + m_faceSize[j - 1];

			width = width >> 1;
			height = height >> 1;
			depth = depth >> 1;

			width = (width == 0) ? 1 : width;
			height = (height == 0) ? 1 : height;
			depth = (depth == 0) ? 1 : depth;
		}
	}
	else
	{
		for (uint32_t i = 0; i < m_depthOrArraySize; ++i)
		{
			uint32_t width = m_width;
			uint32_t height = m_height;
			uint32_t depth = m_depthOrArraySize;

			for (uint32_t j = 0; j < m_numMips; ++j)
			{
				size_t numBytes;
				size_t rowBytes;
				size_t numRows;
				GetSurfaceInfo(width, height, format, numBytes, rowBytes, numRows);

				sizeBytes += numBytes;

				if (i == 0)
				{
					m_platformData->arraySliceSize += numBytes;
					m_faceSize[j] = numBytes;

					m_platformData->faceOffsets[j] = (j == 0) ? 0 : m_platformData->faceOffsets[j - 1] + m_faceSize[j - 1];
				}

				width = width >> 1;
				height = height >> 1;
				depth = depth >> 1;

				width = (width == 0) ? 1 : width;
				height = (height == 0) ? 1 : height;
				depth = (depth == 0) ? 1 : depth;
			}
		}
	}

	m_platformData->data.reset(new byte[sizeBytes]);
	m_platformData->totalSize = sizeBytes;
}


void TextureInitializer::SetData(uint32_t slice, uint32_t mipLevel, const void* data)
{
	size_t srcSize = GetFaceSize(mipLevel);

	size_t offset = slice * m_platformData->arraySliceSize + m_platformData->faceOffsets[mipLevel];
	
	memcpy(m_platformData->data.get() + offset, data, srcSize);
}


void Texture::Create1D(uint32_t width, Format format, const void* initData)
{
	TextureInitializer init(ResourceType::Texture1D, format, width, 1, 1, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create2D(uint32_t width, uint32_t height, Format format, const void* initData)
{
	TextureInitializer init(ResourceType::Texture2D, format, width, height, 1, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create3D(uint32_t width, uint32_t height, uint32_t depth, Format format, const void* initData)
{
	TextureInitializer init(ResourceType::Texture3D, format, width, height, depth, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create(TextureInitializer& init)
{
	m_width = init.m_width;
	m_height = init.m_height;
	m_arraySize = init.m_depthOrArraySize;
	m_format = init.m_format;
	m_numMips = init.m_numMips;
	m_type = init.m_type;

	VkDevice device = GetDevice();

	VkFormat vkFormat = static_cast<VkFormat>(m_format);
	
	VkFormatProperties properties = GetFormatProperties(m_format);
	VkImageUsageFlags additionalFlags = 0;

	if (QueryOptimalTilingFeature(properties, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
	{
		additionalFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = GetImageCreateFlags(m_type);
	imageCreateInfo.imageType = GetImageType(m_type);
	imageCreateInfo.format = vkFormat;
	imageCreateInfo.mipLevels = m_numMips;
	imageCreateInfo.arrayLayers = m_type == ResourceType::Texture3D ? 1 : m_arraySize;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { m_width, m_height, (m_type == ResourceType::Texture3D ? m_arraySize : 1) };
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | additionalFlags;
	// Ensure that the TRANSFER_DST bit is set for staging
	if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}

	VkImage image{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateImage(device, &imageCreateInfo, nullptr, &image));

	VkMemoryRequirements memReqs = {};
	vkGetImageMemoryRequirements(device, image, &memReqs);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkDeviceMemory mem{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(device, &memAllocInfo, nullptr, &mem));
	m_resource = ResourceHandle::Create(image, mem);

	ThrowIfFailed(vkBindImageMemory(device, image, m_resource, 0));

	// Setup buffer copy regions for each mip level
	uint32_t effectiveArraySize = m_type == ResourceType::Texture3D ? 1 : m_arraySize;
	vector<VkBufferImageCopy> bufferCopyRegions(effectiveArraySize * m_numMips);
	uint32_t offset = 0;

	uint32_t index = 0;
	
	for (uint32_t i = 0; i < effectiveArraySize; ++i)
	{
		uint32_t width = m_width;
		uint32_t height = m_height;
		uint32_t depth = m_arraySize;
		for (uint32_t j = 0; j < m_numMips; ++j)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = j;
			bufferCopyRegion.imageSubresource.baseArrayLayer = i;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = width;
			bufferCopyRegion.imageExtent.height = height;
			bufferCopyRegion.imageExtent.depth = m_type == ResourceType::Texture3D ? depth : 1;
			bufferCopyRegion.bufferOffset = offset;

			bufferCopyRegions[index] = bufferCopyRegion;

			size_t numBytes;
			size_t rowBytes;
			size_t numRows;
			GetSurfaceInfo(width, height, vkFormat, numBytes, rowBytes, numRows);

			offset += (uint32_t)numBytes;
			++index;

			width = width >> 1;
			height = height >> 1;
			depth = depth >> 1;

			width = (width == 0) ? 1 : width;
			height = (height == 0) ? 1 : height;
			depth = (depth == 0) ? 1 : depth;
		}
	}

	// Upload to GPU
	CommandContext::InitializeTexture(*this, init.m_platformData->totalSize, init.m_platformData->data.get(), effectiveArraySize * m_numMips, bufferCopyRegions.data());

	CreateDerivedViews();
}


void Texture::LoadDDS(const string& fullpath, bool sRgb)
{
	unique_ptr<byte[]> data;
	size_t dataSize;

	ThrowIfFailed(BinaryReader::ReadEntireFile(fullpath, data, &dataSize));

	auto device = GetDevice();

	assert(false);
	//ThrowIfFailed(CreateDDSTextureFromMemory(device, data.get(), dataSize, 0, sRgb, &m_resource, m_cpuDescriptorHandle));
}


void Texture::LoadKTX(const string& fullpath, bool sRgb)
{
	unique_ptr<byte[]> data;
	size_t dataSize;

	ThrowIfFailed(BinaryReader::ReadEntireFile(fullpath, data, &dataSize));

	auto device = GetDevice();

	ThrowIfFailed(CreateKTXTextureFromMemory(data.get(), dataSize, 0, sRgb, this));
}


void ManagedTexture::WaitForLoad() const
{
	volatile VkImageView& volHandle = (volatile VkImageView&)m_srv.GetHandle();
	volatile bool& volValid = (volatile bool&)m_isValid;
	while (volHandle == VK_NULL_HANDLE && volValid)
	{
		this_thread::yield();
	}
}