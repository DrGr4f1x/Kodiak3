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
#include "GraphicsDeviceVk.h"


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


void Texture::Create1D(uint32_t width, Format format, const void* initData)
{
	CreateInternal(TextureType::Texture1D, width, 1, 1, 1, format, initData);
}


void Texture::Create2D(uint32_t width, uint32_t height, Format format, const void* initData)
{
	CreateInternal(TextureType::Texture2D, width, height, 1, 1, format, initData);
}


void Texture::Create2DArray(uint32_t width, uint32_t height, uint32_t arraySize, Format format, const void* initData)
{
	CreateInternal(TextureType::Texture2D_Array, width, height, arraySize, 1, format, initData);
}


void Texture::CreateCube(uint32_t width, uint32_t height, Format format, const void* initData)
{
	CreateInternal(TextureType::TextureCube, width, height, 6, 1, format, initData);
}


void Texture::Create3D(uint32_t width, uint32_t height, uint32_t depth, Format format, const void* initData)
{
	CreateInternal(TextureType::Texture3D, width, height, depth, 1, format, initData);
}


void Texture::Destroy()
{
	GpuResource::Destroy();

	auto device = GetDevice();

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


void Texture::CreateInternal(TextureType type, uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, Format format, const void* initData)
{
	assert(initData);

	m_width = width;
	m_height = height;
	m_depthOrArraySize = depthOrArraySize;
	m_format = format;
	m_type = type;

	auto device = GetDevice();

	VkFormat vkFormat = static_cast<VkFormat>(format);
	uint32_t size = width * height * depthOrArraySize * BytesPerPixel(format);

	VkFormatProperties properties = GetFormatProperties(format);
	VkImageUsageFlags additionalFlags = 0;

	if (QueryOptimalTilingFeature(properties, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
	{
		additionalFlags |= VK_IMAGE_USAGE_STORAGE_BIT;
	}

	// Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = GetImageCreateFlags(type);
	imageCreateInfo.imageType = GetImageType(type);
	imageCreateInfo.format = vkFormat;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.arrayLayers = type == TextureType::Texture3D ? 1 : depthOrArraySize;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { m_width, m_height, (type == TextureType::Texture3D ? depthOrArraySize : 1) };
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

	ThrowIfFailed(vkAllocateMemory(device, &memAllocInfo, nullptr, &m_deviceMemory));
	ThrowIfFailed(vkBindImageMemory(device, m_image, m_deviceMemory, 0));

	// Upload to GPU
	CommandContext::InitializeTexture(*this, initData, size);

	// Create the image view (SRV)
	VkImageViewCreateInfo colorImageView = {};
	colorImageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	colorImageView.pNext = nullptr;
	colorImageView.flags = 0;
	colorImageView.viewType = GetImageViewType(type);
	colorImageView.format = vkFormat;
	colorImageView.subresourceRange = {};
	colorImageView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	colorImageView.subresourceRange.baseMipLevel = 0;
	colorImageView.subresourceRange.levelCount = 1;
	colorImageView.subresourceRange.baseArrayLayer = 0;
	colorImageView.subresourceRange.layerCount = type == TextureType::Texture3D ? 1 : depthOrArraySize;
	colorImageView.image = m_image;
	ThrowIfFailed(vkCreateImageView(device, &colorImageView, nullptr, &m_imageView));
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