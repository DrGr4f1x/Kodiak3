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

#include "LogicalDeviceVk.h"

#include "InstanceVk.h"
#include "PhysicalDeviceVk.h"
#include "SemaphoreVk.h"


using namespace Kodiak;
using namespace std;


LogicalDevice::LogicalDevice(const shared_ptr<PhysicalDevice>& physicalDevice)
	: Reference(physicalDevice)
{}


LogicalDevice::~LogicalDevice()
{
	vkDestroyDevice(m_device, nullptr);
	m_device = VK_NULL_HANDLE;
}


shared_ptr<LogicalDevice> LogicalDevice::Create(
	const shared_ptr<PhysicalDevice>& physicalDevice, 
	const vector<VkDeviceQueueCreateInfo>& queueCreateInfos, 
	const vector<string>& enabledLayerNames,
	const vector<string>& enabledExtensionNames, 
	const VkPhysicalDeviceFeatures2& enabledFeatures)
{
	shared_ptr<LogicalDevice> device(new LogicalDevice(physicalDevice));
	device->Initialize(queueCreateInfos, enabledLayerNames, enabledExtensionNames, enabledFeatures);
	return device;
}


shared_ptr<Semaphore> LogicalDevice::CreateBinarySemaphore()
{
	return make_shared<Semaphore>(shared_from_this(), SemaphoreType::Binary);
}


shared_ptr<Semaphore> LogicalDevice::CreateTimelineSemaphore()
{
	return make_shared<Semaphore>(shared_from_this(), SemaphoreType::Timeline);
}


void LogicalDevice::Initialize(
	const vector<VkDeviceQueueCreateInfo>& queueCreateInfos, 
	const vector<string>& enabledLayerNames, 
	const vector<string>& enabledExtensionNames, 
	const VkPhysicalDeviceFeatures2& enabledFeatures)
{
	vector<const char*> layerNames;
	layerNames.reserve(enabledLayerNames.size());
	for (const auto& name : enabledLayerNames)
	{
		layerNames.push_back(name.c_str());
	}

	vector<const char*> extensionNames;
	extensionNames.reserve(enabledExtensionNames.size());
	for (const auto& name : enabledExtensionNames)
	{
		extensionNames.push_back(name.c_str());
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &enabledFeatures;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = nullptr;
	createInfo.enabledLayerCount = (uint32_t)layerNames.size();
	createInfo.ppEnabledLayerNames = layerNames.data();
	createInfo.enabledExtensionCount = (uint32_t)extensionNames.size();
	createInfo.ppEnabledExtensionNames = extensionNames.data();

	ThrowIfFailed(vkCreateDevice(*Get(), &createInfo, nullptr, &m_device));
}