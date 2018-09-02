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

#include "TextureVk.h"

#include "BinaryReader.h"
#include "Filesystem.h"
#include "KTXTextureLoader.h"

#include "CommandContextVk.h"
#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


namespace
{

uint32_t BytesPerPixel(Format format)
{
	return BitsPerPixel(format) / 8;
}


map<string, shared_ptr<ManagedTexture>> s_textureCache;


pair<shared_ptr<ManagedTexture>, bool> FindOrLoadTexture(const string& filename)
{
	static mutex s_mutex;
	lock_guard<mutex> CS(s_mutex);

	auto iter = s_textureCache.find(filename);

	// If it's found, it has already been loaded or the load process has begun
	if (iter != s_textureCache.end())
	{
		return make_pair(iter->second, false);
	}

	auto newTexture = make_shared<ManagedTexture>(filename);
	s_textureCache[filename] = newTexture;

	// This was the first time it was requested, so indicate that the caller must read the file
	return make_pair(newTexture, true);
}


template <typename F>
shared_ptr<Texture> MakeTexture(const string& filename, F fn)
{
	auto managedTex = FindOrLoadTexture(filename);

	auto manTex = managedTex.first;
	const bool requestsLoad = managedTex.second;

	if (!requestsLoad)
	{
		manTex->WaitForLoad();
		return manTex;
	}

	fn(manTex.get(), filename);
	return manTex;
}


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


VkImageType GetImageType(TextureType type)
{
	switch (type)
	{
	case TextureType::Texture1D:
	case TextureType::Texture1D_Array:
		return VK_IMAGE_TYPE_1D;
	case TextureType::Texture2D:
	case TextureType::Texture2D_Array:
	case TextureType::TextureCube:
	case TextureType::TextureCube_Array:
		return VK_IMAGE_TYPE_2D;
	case TextureType::Texture3D:
		return VK_IMAGE_TYPE_3D;
	default:
		assert(false);
		return VK_IMAGE_TYPE_2D;
	}
}


VkImageViewType GetImageViewType(TextureType type)
{
	switch (type)
	{
	case TextureType::Texture1D:
		return VK_IMAGE_VIEW_TYPE_1D;
	case TextureType::Texture1D_Array:
		return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
	case TextureType::Texture2D:
		return VK_IMAGE_VIEW_TYPE_2D;
	case TextureType::Texture2D_Array:
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
	case TextureType::TextureCube:
		return VK_IMAGE_VIEW_TYPE_CUBE;
	case TextureType::TextureCube_Array:
		return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
	case TextureType::Texture3D:
		return VK_IMAGE_VIEW_TYPE_3D;
	default:
		assert(false);
		return VK_IMAGE_VIEW_TYPE_2D;
	}
}


VkImageCreateFlagBits GetImageCreateFlags(TextureType type)
{
	switch (type)
	{
	case TextureType::TextureCube:
	case TextureType::TextureCube_Array:
		return VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	default:
		return static_cast<VkImageCreateFlagBits>(0);
	}
}

} // anonymous namespace


void TextureInitializer::Initialize()
{
	size_t sizeBytes = 0;
	auto format = static_cast<VkFormat>(m_format);

	if (m_type == TextureType::Texture3D)
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

			m_faceOffsets[j] = (j == 0) ? 0 : m_faceOffsets[j - 1] + m_faceSize[j - 1];

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
					m_arraySliceSize += numBytes;
					m_faceSize[j] = numBytes;

					m_faceOffsets[j] = (j == 0) ? 0 : m_faceOffsets[j - 1] + m_faceSize[j - 1];
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

	m_data.reset(new byte[sizeBytes]);
	m_totalSize = sizeBytes;
}


void TextureInitializer::SetData(uint32_t slice, uint32_t mipLevel, const void* data)
{
	size_t srcSize = GetFaceSize(mipLevel);

	size_t offset = slice * m_arraySliceSize + m_faceOffsets[mipLevel];
	
	memcpy(m_data.get() + offset, data, srcSize);
}


void Texture::Create1D(uint32_t width, Format format, const void* initData)
{
	TextureInitializer init(TextureType::Texture1D, format, width, 1, 1, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create2D(uint32_t width, uint32_t height, Format format, const void* initData)
{
	TextureInitializer init(TextureType::Texture2D, format, width, height, 1, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create3D(uint32_t width, uint32_t height, uint32_t depth, Format format, const void* initData)
{
	TextureInitializer init(TextureType::Texture3D, format, width, height, depth, 1);
	init.SetData(0, 0, initData);

	Create(init);
}


void Texture::Create(TextureInitializer& init)
{
	m_width = init.m_width;
	m_height = init.m_height;
	m_depthOrArraySize = init.m_depthOrArraySize;
	m_format = init.m_format;
	m_numMips = init.m_numMips;
	m_type = init.m_type;

	VkDevice device = *GetDevice();

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
	imageCreateInfo.arrayLayers = m_type == TextureType::Texture3D ? 1 : m_depthOrArraySize;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { m_width, m_height, (m_type == TextureType::Texture3D ? m_depthOrArraySize : 1) };
	imageCreateInfo.usage = VK_IMAGE_USAGE_SAMPLED_BIT | additionalFlags;
	// Ensure that the TRANSFER_DST bit is set for staging
	if (!(imageCreateInfo.usage & VK_IMAGE_USAGE_TRANSFER_DST_BIT))
	{
		imageCreateInfo.usage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	}
	ThrowIfFailed(vkCreateImage(device, &imageCreateInfo, nullptr, &m_image));

	VkMemoryRequirements memReqs = {};
	vkGetImageMemoryRequirements(device, m_image, &memReqs);

	VkMemoryAllocateInfo memAllocInfo = {};
	memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkDeviceMemory mem{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(device, &memAllocInfo, nullptr, &mem));
	m_resource = CreateHandle(mem);

	ThrowIfFailed(vkBindImageMemory(device, m_image, *m_resource, 0));

	// Setup buffer copy regions for each mip level
	uint32_t effectiveArraySize = m_type == TextureType::Texture3D ? 1 : m_depthOrArraySize;
	vector<VkBufferImageCopy> bufferCopyRegions(effectiveArraySize * m_numMips);
	uint32_t offset = 0;

	uint32_t index = 0;
	
	for (uint32_t i = 0; i < effectiveArraySize; ++i)
	{
		uint32_t width = m_width;
		uint32_t height = m_height;
		uint32_t depth = m_depthOrArraySize;
		for (uint32_t j = 0; j < m_numMips; ++j)
		{
			VkBufferImageCopy bufferCopyRegion = {};
			bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			bufferCopyRegion.imageSubresource.mipLevel = j;
			bufferCopyRegion.imageSubresource.baseArrayLayer = i;
			bufferCopyRegion.imageSubresource.layerCount = 1;
			bufferCopyRegion.imageExtent.width = width;
			bufferCopyRegion.imageExtent.height = height;
			bufferCopyRegion.imageExtent.depth = m_type == TextureType::Texture3D ? depth : 1;
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
	CommandContext::InitializeTexture(*this, init.m_totalSize, init.m_data.get(), effectiveArraySize * m_numMips, bufferCopyRegions.data());

	// Create the image view (SRV)
	VkImageViewCreateInfo colorImageView = {};
	colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageView.pNext = nullptr;
	colorImageView.flags = 0;
	colorImageView.viewType = GetImageViewType(m_type);
	colorImageView.format = vkFormat;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = m_numMips;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = m_type == TextureType::Texture3D ? 1 : m_depthOrArraySize;
	colorImageView.image = m_image;
	ThrowIfFailed(vkCreateImageView(device, &colorImageView, nullptr, &m_imageView));
}


void Texture::Destroy()
{
	// TODO
	m_resource = nullptr;

	VkDevice device = *GetDevice();

	vkDestroyImageView(device, m_imageView, nullptr);
	m_imageView = VK_NULL_HANDLE;

	vkDestroyImage(device, m_image, nullptr);
	m_image = VK_NULL_HANDLE;
}


void Texture::DestroyAll()
{
	s_textureCache.clear();
}


shared_ptr<Texture> Texture::Load(const string& filename, bool sRgb)
{
	auto& filesystem = Filesystem::GetInstance();
	string fullpath = filesystem.GetFullPath(filename);

	assert_msg(!fullpath.empty(), "Could not find texture file %s", filename.c_str());

	string extension = filesystem.GetFileExtension(filename);

	if (extension == ".dds")
	{
		return MakeTexture(fullpath, [sRgb](Texture* texture, const string& filename) { texture->LoadDDS(filename, sRgb); });
	}
	else if (extension == ".ktx")
	{
		return MakeTexture(fullpath, [sRgb](Texture* texture, const string& filename) { texture->LoadKTX(filename, sRgb); });
	}
	else
	{
		assert_msg(false, "Unknown file extension %s", extension.c_str());
		return nullptr;
	}
}


shared_ptr<Texture> Texture::GetBlackTex2D()
{
	auto makeBlackTex = [](Texture* texture, const string& filename)
	{
		uint32_t blackPixel = 0u;
		texture->Create2D(1, 1, Format::R8G8B8A8_UNorm, &blackPixel);
	};

	return MakeTexture("DefaultWhiteTexture", makeBlackTex);
}


shared_ptr<Texture> Texture::GetWhiteTex2D()
{
	auto makeWhiteTex = [](Texture* texture, const string& filename)
	{
		uint32_t whitePixel = 0xFFFFFFFFul;
		texture->Create2D(1, 1, Format::R8G8B8A8_UNorm, &whitePixel);
	};

	return MakeTexture("DefaultWhiteTexture", makeWhiteTex);
}


shared_ptr<Texture> Texture::GetMagentaTex2D()
{
	auto makeMagentaTex = [](Texture* texture, const string& filename)
	{
		uint32_t magentaPixel = 0x00FF00FF;
		texture->Create2D(1, 1, Format::R8G8B8A8_UNorm, &magentaPixel);
	};

	return MakeTexture("DefaultMagentaTexture", makeMagentaTex);
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
	volatile VkImageView& volHandle = (volatile VkImageView&)m_imageView;
	volatile bool& volValid = (volatile bool&)m_isValid;
	while (volHandle == VK_NULL_HANDLE && volValid)
	{
		this_thread::yield();
	}
}


void ManagedTexture::SetToInvalidTexture()
{
	m_imageView = Texture::GetMagentaTex2D()->GetSRV();
	m_isValid = false;
}