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

#include "ResourceView.h"

#include "GraphicsDevice.h"


using namespace Kodiak;


ShaderResourceView::ShaderResourceView() = default;


void ShaderResourceView::Create(const ResourceHandle& resource, ResourceType type, const ResourceViewDesc& desc)
{
	m_handle.buffer = resource;
	m_handle.offset = 0;
	m_handle.range = desc.bufferSize;
}


UnorderedAccessView::UnorderedAccessView() = default;


void UnorderedAccessView::Create(const ResourceHandle& resource, ResourceType type, const ResourceViewDesc& desc)
{
	m_handle.buffer = resource;
	m_handle.offset = 0;
	m_handle.range = desc.bufferSize;
}


IndexBufferView::IndexBufferView() = default;


void IndexBufferView::Create(const ResourceHandle& handle, const ResourceViewDesc& desc)
{
	// Nothing to do for Vulkan
}


VertexBufferView::VertexBufferView() = default;


void VertexBufferView::Create(const ResourceHandle& handle, const ResourceViewDesc& desc)
{
	// Nothing to do for Vulkan
}


ConstantBufferView::ConstantBufferView() = default;


void ConstantBufferView::Create(const ResourceHandle& resource, const ResourceViewDesc& desc)
{
	m_handle.buffer = resource;
	m_handle.offset = 0;
	m_handle.range = desc.bufferSize;
}


DepthStencilView::DepthStencilView() = default;


void DepthStencilView::Create(const ResourceHandle& resource, const DepthStencilViewDesc& desc)
{
	auto vkFormat = static_cast<VkFormat>(desc.format);

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
	imageViewCreateInfo.image = resource;

	VkImageView dsv{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateImageView(GetDevice(), &imageViewCreateInfo, nullptr, &dsv));
	m_handle = DsvHandle::Create(dsv);
}