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

#include "DepthBufferVk.h"

#include "GraphicsDeviceVk.h"


using namespace Kodiak;


void DepthBuffer::Destroy()
{
	if (m_dsv != VK_NULL_HANDLE)
	{
		vkDestroyImageView(GetDevice(), m_dsv, nullptr);
		m_dsv = VK_NULL_HANDLE;
	}

	PixelBuffer::Destroy();
}


void DepthBuffer::Create(const std::string& name, uint32_t width, uint32_t height, Format format)
{
	m_format = format;

	auto imageCreateInfo = DescribeTex2D(width, height, 1, 1, format, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
	CreateTextureResource(name, imageCreateInfo);
	CreateDerivedViews(format);
}


void DepthBuffer::CreateDerivedViews(Format format)
{
	auto vkFormat = static_cast<VkFormat>(format);

	VkImageAspectFlags flags = VK_IMAGE_ASPECT_DEPTH_BIT;
	if (vkFormat == VK_FORMAT_D24_UNORM_S8_UINT || vkFormat == VK_FORMAT_D32_SFLOAT_S8_UINT)
	{
		flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
	}

	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.pNext = nullptr;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = vkFormat;
	imageViewCreateInfo.flags = 0;
	imageViewCreateInfo.subresourceRange = {};
	imageViewCreateInfo.subresourceRange.aspectMask = flags;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	imageViewCreateInfo.image = m_image;

	ThrowIfFailed(vkCreateImageView(GetDevice(), &imageViewCreateInfo, nullptr, &m_dsv));
}