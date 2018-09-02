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


template<> VkHandle<VkDeviceMemory>::~VkHandle()
{
	if (m_wrapped != VK_NULL_HANDLE)
	{
		vkFreeMemory(GetDevice(), m_wrapped, nullptr);
		m_wrapped = VK_NULL_HANDLE;
	}
}


// Instantiate templates to avoid linker issues
template VkHandle<VkInstance>::~VkHandle();
template VkHandle<VkDevice>::~VkHandle();
template VkHandle<VkDeviceMemory>::~VkHandle();

} // namespace Kodiak