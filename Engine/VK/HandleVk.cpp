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

#include "HandleVk.h"

#include "GraphicsDevice.h"


namespace Kodiak
{

VkResourceHandle::~VkResourceHandle()
{
	VkDevice device = GetDevice();

	if (m_isImage)
	{
		if (m_ownsImage && (m_wrapped.image != VK_NULL_HANDLE))
		{
			vkDestroyImage(device, m_wrapped.image, nullptr);
			m_wrapped.image = VK_NULL_HANDLE;
		}
	}
	else
	{
		if (m_wrapped.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(device, m_wrapped.buffer, nullptr);
			m_wrapped.buffer = VK_NULL_HANDLE;
		}
	}

	if (m_wrappedMemory != VK_NULL_HANDLE)
	{
		vkFreeMemory(device, m_wrappedMemory, nullptr);
		m_wrappedMemory = VK_NULL_HANDLE;
	}
}


VkDescriptorHandle::~VkDescriptorHandle()
{
	auto device = GetDevice();

	if (m_isImageView && (m_wrapped.imageView != VK_NULL_HANDLE))
	{
		vkDestroyImageView(device, m_wrapped.imageView, nullptr);
		m_wrapped.imageView = VK_NULL_HANDLE;
	}
	else if (m_isBufferView && (m_wrapped.bufferView != VK_NULL_HANDLE))
	{
		vkDestroyBufferView(device, m_wrapped.bufferView, nullptr);
		m_wrapped.bufferView = VK_NULL_HANDLE;
	}
	else
	{
		m_wrapped.buffer = VK_NULL_HANDLE;
	}
}


VkFramebufferHandle::~VkFramebufferHandle()
{
	auto device = GetDevice();

	if (m_renderPass != VK_NULL_HANDLE)
	{
		vkDestroyRenderPass(device, m_renderPass, nullptr);
		m_renderPass = VK_NULL_HANDLE;
	}

	if (m_framebuffer != VK_NULL_HANDLE)
	{
		vkDestroyFramebuffer(device, m_framebuffer, nullptr);
		m_framebuffer = VK_NULL_HANDLE;
	}
}


template<> VkHandle<VkInstance>::~VkHandle()
{
	if (m_wrapped != VK_NULL_HANDLE)
	{
		vkDestroyInstance(m_wrapped, nullptr);
		m_wrapped = VK_NULL_HANDLE;
	}
}


template<> VkHandle<VkDevice>::~VkHandle()
{
	if (m_wrapped != VK_NULL_HANDLE)
	{
		vkDestroyDevice(m_wrapped, nullptr);
		m_wrapped = VK_NULL_HANDLE;
	}
}


template<> VkHandle<VkImageView>::~VkHandle()
{
	if (m_wrapped != VK_NULL_HANDLE)
	{
		vkDestroyImageView(GetDevice(), m_wrapped, nullptr);
		m_wrapped = VK_NULL_HANDLE;
	}
}


// Instantiate templates to avoid linker issues
template VkHandle<VkInstance>::~VkHandle();
template VkHandle<VkDevice>::~VkHandle();
template VkHandle<VkImageView>::~VkHandle();

} // namespace Kodiak