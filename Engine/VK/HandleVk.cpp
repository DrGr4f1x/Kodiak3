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


// Instantiate templates to avoid linker issues
template VkHandle<VkInstance>::~VkHandle();
template VkHandle<VkDevice>::~VkHandle();

} // namespace Kodiak