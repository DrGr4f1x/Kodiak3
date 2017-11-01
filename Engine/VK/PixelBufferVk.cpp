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

#include "GraphicsDeviceVk.h"


using namespace Kodiak;


void PixelBuffer::Destroy()
{
	if (m_image != VK_NULL_HANDLE)
	{
		vkDestroyImage(GetDevice(), m_image, nullptr);
		m_image = VK_NULL_HANDLE;
	}

	GpuResource::Destroy();
}


VkImageCreateInfo PixelBuffer::DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, Format format, VkImageUsageFlags usageFlags)
{
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = static_cast<VkFormat>(format);
	imageCreateInfo.extent = { width, height, 1 };
	imageCreateInfo.mipLevels = numMips;
	imageCreateInfo.arrayLayers = depthOrArraySize;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
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

	auto device = GetDevice();

	ThrowIfFailed(vkCreateImage(device, &imageCreateInfo, nullptr, &m_image));

	VkMemoryRequirements memReqs = {};
	vkGetImageMemoryRequirements(device, m_image, &memReqs);

	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.pNext = nullptr;
	memoryAllocateInfo.allocationSize = memReqs.size;
	memoryAllocateInfo.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	ThrowIfFailed(vkAllocateMemory(device, &memoryAllocateInfo, nullptr, &m_deviceMemory));

	ThrowIfFailed(vkBindImageMemory(device, m_image, m_deviceMemory, 0));

	SetDebugName(m_image, name + " image");
	SetDebugName(m_deviceMemory, name + " memory");
}