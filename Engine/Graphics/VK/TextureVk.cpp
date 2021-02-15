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
#include "Graphics\Resources\KTXTextureLoader.h"

#include "CommandContextVk.h"
#include "GraphicsDeviceVk.h"
#include "UtilVk.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


using namespace Kodiak;
using namespace std;


namespace
{

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

} // anonymous namespace


namespace Kodiak
{

struct TextureInitializer::PlatformData
{
	PlatformData(uint32_t numMips) : faceOffsets(numMips) {}

	unique_ptr<uint8_t[]>	data;
	vector<size_t>			faceOffsets;
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

	m_platformData->data.reset(new uint8_t[sizeBytes]);
	m_platformData->totalSize = sizeBytes;
}


void TextureInitializer::SetData(uint32_t slice, uint32_t mipLevel, const void* data)
{
	size_t srcSize = GetFaceSize(mipLevel);

	size_t offset = slice * m_platformData->arraySliceSize + m_platformData->faceOffsets[mipLevel];
	
	memcpy(m_platformData->data.get() + offset, data, srcSize);
}


Texture::Texture() = default;


Texture::~Texture()
{
	g_graphicsDevice->ReleaseResource(m_image.Get());
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
	m_numSamples = 1;

	VkFormat vkFormat = static_cast<VkFormat>(m_format);

	ImageDesc desc = {};
	desc.width = m_width;
	desc.height = m_height;
	desc.depthOrArraySize = m_arraySize;
	desc.numMips = m_numMips;
	desc.numSamples = m_numSamples;
	desc.format = m_format;
	desc.type = m_type;
	desc.usage = GpuImageUsage::ShaderResource | GpuImageUsage::UnorderedAccess | GpuImageUsage::CopyDest;
	desc.access = MemoryAccess::GpuRead | MemoryAccess::GpuWrite;

	ThrowIfFailed(g_graphicsDevice->CreateImage("Texture", desc, &m_image));

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


shared_ptr<Texture> Texture::Load(const string& filename, Format format, bool sRgb)
{
	auto& filesystem = Filesystem::GetInstance();
	string fullpath = filesystem.GetFullPath(filename);

	assert_msg(!fullpath.empty(), "Could not find texture file %s", filename.c_str());

	string extension = filesystem.GetFileExtension(filename);

	if (extension == ".dds")
	{
		return MakeTexture(fullpath, [format, sRgb](Texture* texture, const string& filename) { texture->LoadDDS(filename, format, sRgb); });
	}
	else if (extension == ".ktx")
	{
		return MakeTexture(fullpath, [format, sRgb](Texture* texture, const string& filename) { texture->LoadKTX(filename, format, sRgb); });
	}
	else
	{
		return MakeTexture(fullpath, [format, sRgb](Texture* texture, const string& filename) { texture->LoadTexture(filename, format, sRgb); });
	}
}


shared_ptr<Texture> Texture::GetBlackTex2D()
{
	auto makeBlackTex = [](Texture* texture, const string& filename)
	{
		uint32_t blackPixel = 0u;
		texture->Create2D(1, 1, Format::R8G8B8A8_UNorm, &blackPixel);
	};

	return MakeTexture("DefaultBlackTexture", makeBlackTex);
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


void Texture::DestroyAll()
{
	s_textureCache.clear();
}


void Texture::LoadDDS(const string& fullpath, Format format, bool sRgb)
{
	ThrowIfFailed(BinaryReader::ReadEntireFile(fullpath, m_data, &m_dataSize));

	auto device = GetDevice();

	assert(false);
	//ThrowIfFailed(CreateDDSTextureFromMemory(device, m_data.get(), m_dataSize, 0, format, sRgb, &m_resource, m_cpuDescriptorHandle));

	ClearRetainedData();
}


void Texture::LoadKTX(const string& fullpath, Format format, bool sRgb)
{
	ThrowIfFailed(BinaryReader::ReadEntireFile(fullpath, m_data, &m_dataSize));

	auto device = GetDevice();

	ThrowIfFailed(CreateKTXTextureFromMemory(m_data.get(), m_dataSize, 0, format, sRgb, this));

	ClearRetainedData();
}


void Texture::LoadTexture(const string& fullpath, Format format, bool sRgb)
{
	int x, y, n;
	unsigned char* data = stbi_load(fullpath.c_str(), &x, &y, &n, 4);
	assert_msg(data != nullptr, "Failed to load image %s", fullpath.c_str());

	if (format == Format::Unknown)
	{
		if (n == 1)
			format = Format::R8_UNorm;
		else if (n == 2)
			format = Format::R8G8_UNorm;
		else if (n == 3 || n == 4)
			format = Format::R8G8B8A8_UNorm;
	}

	Create2D(x, y, format, data);

	if (m_retainData)
	{
		m_dataSize = x * y * n;
		m_data.reset(new uint8_t[m_dataSize]);
		memcpy(m_data.get(), data, m_dataSize);
	}

	stbi_image_free(data);
}


void Texture::ClearRetainedData()
{
	if (!m_retainData)
	{
		m_data.reset();
		m_dataSize = 0;
	}
}


void Texture::CreateDerivedViews()
{
	ThrowIfFailed(g_graphicsDevice->CreateImageView(m_image.Get(), m_type, m_format, ImageAspect::Color, 0, m_numMips, 0, m_arraySize, &m_imageView));
	m_imageInfo = { VK_NULL_HANDLE, m_imageView->Get(), GetImageLayout(ResourceState::ShaderResource) };
}


void ManagedTexture::WaitForLoad() const
{
	volatile VkImageView& volHandle = (volatile VkImageView&)m_imageView->GetRef();
	volatile bool& volValid = (volatile bool&)m_isValid;
	while (volHandle == VK_NULL_HANDLE && volValid)
	{
		this_thread::yield();
	}
}


uint32_t Kodiak::BytesPerPixel(Format format)
{
	return BitsPerPixel(format) / 8;
}