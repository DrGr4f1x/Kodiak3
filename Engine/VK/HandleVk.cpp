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

#include "GraphicsDeviceVk.h"


namespace Kodiak
{

template<> VkHandle<VkDeviceMemory>::~VkHandle()
{
	if (m_wrapped != VK_NULL_HANDLE)
	{
		vkFreeMemory(GetDevice(), m_wrapped, nullptr);
		m_wrapped = VK_NULL_HANDLE;
	}
}

template VkHandle<VkDeviceMemory>::~VkHandle();

} // namespace Kodiak