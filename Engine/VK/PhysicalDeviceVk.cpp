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
#include "LogicalDeviceVk.h"
#include "SurfaceVk.h"


using namespace Kodiak;
using namespace std;


PhysicalDevice::PhysicalDevice(const shared_ptr<Instance>& instance, VkPhysicalDevice physicalDevice)
	: Reference(instance)
	, m_physicalDevice(physicalDevice)
	, m_deviceProperties()
	, m_memoryProperties()
	, m_extensions()
	, m_queueFamilyProperties()
{
	m_deviceFeatures2.pNext = &m_deviceFeatures1_2;

	Initialize();
}


PhysicalDevice::~PhysicalDevice()
{
	m_physicalDevice = VK_NULL_HANDLE;
}


shared_ptr<LogicalDevice> PhysicalDevice::CreateLogicalDevice(
	const vector<VkDeviceQueueCreateInfo>& queueCreateInfos, 
	const vector<string>& enabledLayerNames, 
	const vector<string>& enabledExtensionNames, 
	const VkPhysicalDeviceFeatures2& enabledFeatures)
{
	return LogicalDevice::Create(shared_from_this(), queueCreateInfos, enabledLayerNames, enabledExtensionNames, enabledFeatures);
}


bool PhysicalDevice::GetSurfaceSupport(uint32_t index, const shared_ptr<Surface>& surface) const
{
	VkBool32 supported = VK_FALSE;
	ThrowIfFailed(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, index, *surface.get(), &supported));
	return supported == VK_TRUE;
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

	// Get available queue family properties
	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueCount, nullptr);
	assert(queueCount >= 1);

	m_queueFamilyProperties.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueCount, m_queueFamilyProperties.data());
}


namespace Kodiak
{
vector<uint32_t> GetGraphicsPresentQueueFamilyIndices(const shared_ptr<PhysicalDevice>& physicalDevice, const shared_ptr<Surface>& surface)
{
	const auto& properties = physicalDevice->GetQueueFamilyProperties();
	assert(!properties.empty());

	vector<uint32_t> indices;
	for (size_t i = 0; i < properties.size(); ++i)
	{
		if ((properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && physicalDevice->GetSurfaceSupport((uint32_t)i, surface))
		{
			indices.push_back((uint32_t)i);
		}
	}
	return indices;
}


vector<uint32_t> GetQueueFamilyIndices(const shared_ptr<PhysicalDevice>& physicalDevice, VkQueueFlags queueFlags)
{
	const auto& properties = physicalDevice->GetQueueFamilyProperties();
	assert(!properties.empty());

	vector<uint32_t> indices;
	for (size_t i = 0; i < properties.size(); ++i)
	{
		if ((properties[i].queueFlags & queueFlags) == queueFlags)
		{
			indices.push_back((uint32_t)i);
		}
	}
	return indices;
}


uint32_t GetQueueFamilyIndex(const shared_ptr<PhysicalDevice>& physicalDevice, VkQueueFlags queueFlags)
{
	const auto& queueFamilyProperties = physicalDevice->GetQueueFamilyProperties();
	assert(!queueFamilyProperties.empty());

	uint32_t index = 0;

	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queueFlags & VK_QUEUE_COMPUTE_BIT)
	{
		for (const auto& properties : queueFamilyProperties)
		{
			if ((properties.queueFlags & queueFlags) && ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0))
			{
				return index;
			}
			++index;
		}
	}

	// Dedicated queue for transfer
	// Try to find a queue family index that supports transfer but not graphics and compute
	if (queueFlags & VK_QUEUE_TRANSFER_BIT)
	{
		index = 0;
		for (const auto& properties : queueFamilyProperties)
		{
			if ((properties.queueFlags & queueFlags) && ((properties.queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((properties.queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
			{
				return index;
			}
			++index;
		}
	}

	// For other queue types or if no separate compute queue is present, return the first one to support the requested flags
	index = 0;
	for (const auto& properties : queueFamilyProperties)
	{
		if (properties.queueFlags & queueFlags)
		{
			return index;
		}
		++index;
	}

	throw runtime_error("Could not find a matching queue family index");
}

} // namespace Kodiak