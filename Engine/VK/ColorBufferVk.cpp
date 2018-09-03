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

#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


ColorBuffer::~ColorBuffer()
{
	vkDestroyImageView(GetDevice(), m_imageView, nullptr);
	m_imageView = VK_NULL_HANDLE;
	
	g_graphicsDevice->ReleaseResource(m_resource);
}


void ColorBuffer::CreateFromSwapChain(const string& name, VkImage baseImage, uint32_t width, uint32_t height, Format format)
{
	m_resource = ResourceHandle::Create(baseImage, VK_NULL_HANDLE, false);
	
	// TODO
	//SetDebugName(m_image, name);

	m_width = width;
	m_height = height;
	m_format = format;
	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	CreateDerivedViews(format, 1);
}


void ColorBuffer::Create(const string& name, uint32_t width, uint32_t height, uint32_t numMips, Format format)
{
	numMips = (numMips == 0 ? ComputeNumMips(width, height) : numMips);

	m_width = width;
	m_height = height;
	m_format = format;
	m_layout = VK_IMAGE_LAYOUT_UNDEFINED;

	const uint32_t arraySize = 1;
	const uint32_t numSamples = 1;
	const VkImageUsageFlags flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT;

	auto imageCreateInfo = DescribeTex2D(width, height, arraySize, numMips, numSamples, format, flags);
	m_width = imageCreateInfo.extent.width;
	m_height = imageCreateInfo.extent.height;
	m_arraySize = imageCreateInfo.arrayLayers;

	m_resource = CreateTextureResource(name, imageCreateInfo);

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