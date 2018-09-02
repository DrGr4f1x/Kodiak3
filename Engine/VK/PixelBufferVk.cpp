// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "PixelBufferVk.h"

#include "GraphicsDevice.h"


using namespace Kodiak;


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


void PixelBuffer::Destroy()
{
	if (m_image != VK_NULL_HANDLE)
	{
		vkDestroyImage(GetDevice(), m_image, nullptr);
		m_image = VK_NULL_HANDLE;
	}

	// TODO
	m_resource = nullptr;
}


VkImageCreateInfo PixelBuffer::DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, 
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


void PixelBuffer::CreateTextureResource(const std::string& name, const VkImageCreateInfo& imageCreateInfo)
{
	m_width = imageCreateInfo.extent.width;
	m_height = imageCreateInfo.extent.height;
	m_arraySize = imageCreateInfo.arrayLayers;

	VkDevice device = GetDevice();

	ThrowIfFailed(vkCreateImage(device, &imageCreateInfo, nullptr, &m_image));

	VkMemoryRequirements memReqs = {};
	vkGetImageMemoryRequirements(device, m_image, &memReqs);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memReqs.size;
	memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkDeviceMemory mem{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &mem));
	m_resource = ResourceHandle::Create(mem);

	ThrowIfFailed(vkBindImageMemory(device, m_image, m_resource, 0));

	// TODO
	//SetDebugName(m_image, name + " image");
	//SetDebugName(*m_resource, name + " memory");
}