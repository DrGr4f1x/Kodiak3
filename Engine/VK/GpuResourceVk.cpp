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

#include "GpuResourceVk.h"

#include "GraphicsDeviceVk.h"


using namespace Kodiak;


GpuResource::GpuResource(VkDeviceMemory memory)
	: m_deviceMemory(memory)
{}


void GpuResource::Destroy()
{
	vkFreeMemory(GetDevice(), m_deviceMemory, nullptr);
	m_deviceMemory = VK_NULL_HANDLE;
}