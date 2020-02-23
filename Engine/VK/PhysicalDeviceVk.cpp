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

#include "PhysicalDeviceVk.h"

#include "GraphicsFeatures.h"


using namespace Kodiak;
using namespace std;


PhysicalDevice::PhysicalDevice(const shared_ptr<Instance>& instance, VkPhysicalDevice physicalDevice)
	: Reference<Instance>(instance)
	, m_physicalDevice(physicalDevice)
	, m_deviceProperties()
	, m_memoryProperties()
	, m_extensions()
{
	m_deviceFeatures2.pNext = &m_deviceFeatures1_2;

	Initialize();
}


PhysicalDevice::~PhysicalDevice()
{
	m_physicalDevice = VK_NULL_HANDLE;
}


uint32_t PhysicalDevice::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
{
	for (uint32_t i = 0; i < m_memoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((m_memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				if (memTypeFound)
				{
					*memTypeFound = true;
				}
				return i;
			}
		}
		typeBits >>= 1;
	}

	if (memTypeFound)
	{
		*memTypeFound = false;
		return 0;
	}
	else
	{
		throw runtime_error("Could not find a matching memory type");
	}
}


bool PhysicalDevice::IsExtensionSupported(const string& name) const
{
	const char* c_str = name.c_str();
	return find_if(
		m_extensions.begin(),
		m_extensions.end(),
		[c_str](auto const& e) { return strcmp(e.extensionName, c_str) == 0; }) != m_extensions.end();
}


void PhysicalDevice::Initialize()
{
	// Get device and memory properties
	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_deviceProperties);
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

	// Get device features
	vkGetPhysicalDeviceFeatures2(m_physicalDevice, &m_deviceFeatures2);

	// Get list of supported extensions
	uint32_t extCount = 0;
	ThrowIfFailed(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr));
	if (extCount > 0)
	{
		m_extensions.resize(extCount);
		ThrowIfFailed(vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, &m_extensions.front()));
	}
}