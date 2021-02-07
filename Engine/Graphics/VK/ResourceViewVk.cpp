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

#include "Graphics\ResourceView.h"

#include "Graphics\GraphicsDevice.h"

#include "UtilVk.h"

using namespace Kodiak;


ShaderResourceView::ShaderResourceView() = default;


void ShaderResourceView::Create(const ResourceHandle& resource, ResourceType type, const TextureViewDesc& desc)
{
	// TODO - Validate that ResourceType is compatible with a texture SRV

	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.viewType = GetImageViewType(type);
	createInfo.format = static_cast<VkFormat>(desc.format);
	createInfo.subresourceRange = {};
	createInfo.subresourceRange.aspectMask = GetAspectFlagsFromFormat(desc.format);
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = desc.mipCount;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = type == ResourceType::Texture3D ? 1 : desc.arraySize;
	createInfo.image = resource;

	VkImageView imageView{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateImageView(GetDevice(), &createInfo, nullptr, &imageView));

	m_handle = SrvHandle::Create(imageView, GetImageLayout(desc.usage));
}


void ShaderResourceView::Create(const ResourceHandle& resource, ResourceType type, const BufferViewDesc& desc)
{
	// TODO - Validate that ResourceType is compatible with a buffer SRV

	m_handle = SrvHandle::Create(resource, 0, desc.bufferSize);
}


UnorderedAccessView::UnorderedAccessView() = default;


void UnorderedAccessView::Create(const ResourceHandle& resource, ResourceType type, const TextureViewDesc& desc)
{
	// TODO - Validate that ResourceType is compatible with a texture UAV
	
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.viewType = GetImageViewType(type);
	createInfo.format = static_cast<VkFormat>(desc.format);
	createInfo.subresourceRange = {};
	createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = desc.mipCount;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = type == ResourceType::Texture3D ? 1 : desc.arraySize;
	createInfo.image = resource;

	VkImageView imageView{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateImageView(GetDevice(), &createInfo, nullptr, &imageView));

	m_handle = UavHandle::Create(imageView, GetImageLayout(desc.usage));
}


void UnorderedAccessView::Create(const ResourceHandle& resource, ResourceType type, const BufferViewDesc& desc)
{
	// TODO - Validate that ResourceType is compatible with a buffer UAV

	m_handle = UavHandle::Create(resource, 0, desc.bufferSize);
}


void UnorderedAccessView::Create(const ResourceHandle& resource, ResourceType type, const TypedBufferViewDesc& desc)
{
	// TODO - Validate that ResourceType is compatible with a typed-buffer UAV

	VkBufferViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.buffer = resource;
	createInfo.offset = 0;
	createInfo.range = desc.bufferSize;
	createInfo.format = static_cast<VkFormat>(desc.format);

	VkBufferView bufferView{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateBufferView(GetDevice(), &createInfo, nullptr, &bufferView));

	m_handle = UavHandle::Create(bufferView);
}

IndexBufferView::IndexBufferView() = default;


void IndexBufferView::Create(const ResourceHandle& handle, const BufferViewDesc& desc)
{
	// Nothing to do for Vulkan
}


VertexBufferView::VertexBufferView() = default;


void VertexBufferView::Create(const ResourceHandle& handle, const BufferViewDesc& desc)
{
	// Nothing to do for Vulkan
}


ConstantBufferView::ConstantBufferView() = default;


void ConstantBufferView::Create(const ResourceHandle& resource, const BufferViewDesc& desc)
{
	m_handle = CbvHandle::Create(resource, 0, desc.bufferSize);
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


RenderTargetView::RenderTargetView() = default;


void RenderTargetView::Create(const ResourceHandle& resource, const RenderTargetViewDesc& desc)
{
	VkImageViewCreateInfo imageViewInfo{ VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	imageViewInfo.pNext = nullptr;
	imageViewInfo.flags = 0;
	imageViewInfo.image = resource;
	imageViewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewInfo.format = static_cast<VkFormat>(desc.format);
	imageViewInfo.components.r = VK_COMPONENT_SWIZZLE_R;
	imageViewInfo.components.g = VK_COMPONENT_SWIZZLE_G;
	imageViewInfo.components.b = VK_COMPONENT_SWIZZLE_B;
	imageViewInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	imageViewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewInfo.subresourceRange.baseMipLevel = 0;
	imageViewInfo.subresourceRange.levelCount = desc.numMips;
	imageViewInfo.subresourceRange.baseArrayLayer = 0;
	imageViewInfo.subresourceRange.layerCount = desc.arraySize;

	VkImageView rtv{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateImageView(GetDevice(), &imageViewInfo, nullptr, &rtv));
	m_handle = RtvHandle::Create(rtv);
}