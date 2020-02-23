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

#include "InstanceVk.h"

#include "PhysicalDeviceVk.h"


using namespace Kodiak;
using namespace std;


Instance::Instance(const VkInstanceCreateInfo& createInfo)
{
	Initialize(createInfo);
}


Instance::~Instance()
{
	assert(m_instance != VK_NULL_HANDLE);
	
	vkDestroyInstance(m_instance, nullptr);
	m_instance = VK_NULL_HANDLE;
}


shared_ptr<Instance> Instance::Create(const string& appName)
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName.c_str();
	appInfo.pEngineName = "Kodiak";
	appInfo.apiVersion = VK_API_VERSION_1_2;

	vector<const char*> instanceExtensions =
	{
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.pNext = nullptr;
	instanceCreateInfo.pApplicationInfo = &appInfo;
	if (!instanceExtensions.empty())
	{
#if ENABLE_VULKAN_VALIDATION
		instanceExtensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif
		instanceCreateInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensions.data();
	}

#if ENABLE_VULKAN_VALIDATION
	// On desktop the LunarG loaders exposes a meta layer that contains all layers
	int32_t validationLayerCount = 1;
	const char* validationLayerNames[] = {
		"VK_LAYER_KHRONOS_validation"
	};

	instanceCreateInfo.enabledLayerCount = validationLayerCount;
	instanceCreateInfo.ppEnabledLayerNames = validationLayerNames;
#endif

	return make_shared<Instance>(instanceCreateInfo);
}


shared_ptr<PhysicalDevice> Instance::GetPhysicalDevice(size_t index)
{
	assert(index < m_physicalDevices.size());

	auto physicalDevice = m_cachedPhysicalDevices[index].lock();
	if (!physicalDevice)
	{
		physicalDevice = make_shared<PhysicalDevice>(shared_from_this(), m_physicalDevices[index]);
	}

	return physicalDevice;
}


void Instance::Initialize(const VkInstanceCreateInfo& createInfo)
{
	auto res = vkCreateInstance(&createInfo, nullptr, &m_instance);
	if (VK_SUCCESS != res)
	{
		Utility::ExitFatal("Could not create Vulkan instance", "Fatal error");
	}

	EnumeratePhysicalDevices();
	m_cachedPhysicalDevices.resize(GetPhysicalDeviceCount());
}


void Instance::EnumeratePhysicalDevices()
{
	uint32_t gpuCount = 0;
	// Get number of available physical devices
	ThrowIfFailed(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
	assert(gpuCount > 0);

	// Enumerate devices
	m_physicalDevices.resize(gpuCount);
	auto res = vkEnumeratePhysicalDevices(m_instance, &gpuCount, m_physicalDevices.data());
	if (res)
	{
		Utility::ExitFatal("Could not enumerate physical devices", "Fatal error");
	}
}