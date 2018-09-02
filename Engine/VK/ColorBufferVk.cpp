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

#include "ColorBufferVk.h"

#include "GraphicsDevice.h"

using namespace Kodiak;


void ColorBuffer::Destroy()
{
	vkDestroyImageView(GetDevice(), m_imageView, nullptr);
	m_imageView = VK_NULL_HANDLE;
	
	if (m_ownsImage)
	{
		PixelBuffer::Destroy();
	}
}


void ColorBuffer::CreateFromSwapChain(const std::string& name, VkImage baseImage, uint32_t width, uint32_t height, Format format)
{
	m_ownsImage = false;

	m_resource = ResourceHandle::Create(baseImage, VK_NULL_HANDLE, false);
	
	// TODO
	//SetDebugName(m_image, name);

	m_width = width;
	m_height = height;
	m_format = format;
	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	CreateDerivedViews(format, 1);
}


void ColorBuffer::Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numMips, Format format)
{
	numMips = (numMips == 0 ? ComputeNumMips(width, height) : numMips);

	m_ownsImage = true;

	m_width = width;
	m_height = height;
	m_format = format;
	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	const uint32_t arraySize = 1;
	const uint32_t numSamples = 1;
	const VkImageUsageFlags flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

	auto imageCreateInfo = DescribeTex2D(width, height, arraySize, numMips, numSamples, format, flags);
	CreateTextureResource(name, imageCreateInfo);
	CreateDerivedViews(format, 1, numMips);
}


void ColorBuffer::CreateDerivedViews(Format format, uint32_t arraySize, uint32_t numMips)
{
	m_numMipMaps = numMips - 1;

	VkImageViewCreateInfo imageViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewInfo.pNext = nullptr;
	imageViewInfo.flags = 0;
	imageViewInfo.image = m_resource;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = static_cast<VkFormat>(format);
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = numMips;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = arraySize;

	ThrowIfFailed(vkCreateImageView(GetDevice(), &imageViewInfo, nullptr, &m_imageView));
}