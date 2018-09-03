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

#include "UtilVk.h"

#include "GraphicsDevice.h"


namespace Kodiak
{

VkImageCreateInfo DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips,
	uint32_t numSamples, Format format, VkImageUsageFlags usageFlags)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = static_cast<VkFormat>(format);
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = numMips;
	imageCreateInfo.arrayLayers = depthOrArraySize;
	imageCreateInfo.samples = SamplesToFlags(numSamples);
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = usageFlags;
	imageCreateInfo.flags = 0;

	return imageCreateInfo;
}


ResourceHandle CreateTextureResource(const std::string& name, const VkImageCreateInfo& imageCreateInfo)
{
	VkDevice device = GetDevice();

	VkImage image{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateImage(device, &imageCreateInfo, nullptr, &image));

	VkMemoryRequirements memReqs = {};
	vkGetImageMemoryRequirements(device, image, &memReqs);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memReqs.size;
	memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkDeviceMemory mem{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &mem));
	ResourceHandle handle = ResourceHandle::Create(image, mem);

	ThrowIfFailed(vkBindImageMemory(device, image, handle, 0));

	// TODO
	//SetDebugName(m_image, name + " image");
	//SetDebugName(*m_resource, name + " memory");

	return handle;
}


VkSampleCountFlagBits Kodiak::SamplesToFlags(uint32_t numSamples)
{
	switch (numSamples)
	{
	case 1:
		return VK_SAMPLE_COUNT_1_BIT;
	case 2:
		return VK_SAMPLE_COUNT_2_BIT;
	case 4:
		return VK_SAMPLE_COUNT_4_BIT;
	case 8:
		return VK_SAMPLE_COUNT_8_BIT;
	case 16:
		return VK_SAMPLE_COUNT_16_BIT;
	case 32:
		return VK_SAMPLE_COUNT_32_BIT;
	case 64:
		return VK_SAMPLE_COUNT_64_BIT;
	default:
		assert(false);
		return VK_SAMPLE_COUNT_1_BIT;
	}
}

VkImageType GetImageType(ResourceType type)
{
	switch (type)
	{
	case ResourceType::Texture1D:
	case ResourceType::Texture1D_Array:
		return VK_IMAGE_TYPE_1D;

	case ResourceType::Texture2D:
	case ResourceType::Texture2D_Array:
	case ResourceType::Texture2DMS:
	case ResourceType::Texture2DMS_Array:
	case ResourceType::TextureCube:
	case ResourceType::TextureCube_Array:
		return VK_IMAGE_TYPE_2D;

	case ResourceType::Texture3D:
		return VK_IMAGE_TYPE_3D;

	default:
		assert(false);
		return VK_IMAGE_TYPE_2D;
	}
}


VkImageViewType GetImageViewType(ResourceType type)
{
	switch (type)
	{
	case ResourceType::Texture1D:
		return VK_IMAGE_VIEW_TYPE_1D;

	case ResourceType::Texture1D_Array:
		return VK_IMAGE_VIEW_TYPE_1D_ARRAY;

	case ResourceType::Texture2D:
	case ResourceType::Texture2DMS:
		return VK_IMAGE_VIEW_TYPE_2D;

	case ResourceType::Texture2D_Array:
	case ResourceType::Texture2DMS_Array:
		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;

	case ResourceType::TextureCube:
		return VK_IMAGE_VIEW_TYPE_CUBE;

	case ResourceType::TextureCube_Array:
		return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

	case ResourceType::Texture3D:
		return VK_IMAGE_VIEW_TYPE_3D;

	default:
		assert(false);
		return VK_IMAGE_VIEW_TYPE_2D;
	}
}


VkImageCreateFlagBits GetImageCreateFlags(ResourceType type)
{
	switch (type)
	{
	case ResourceType::TextureCube:
	case ResourceType::TextureCube_Array:
		return VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
	default:
		return static_cast<VkImageCreateFlagBits>(0);
	}
}

} // namespace Kodiak