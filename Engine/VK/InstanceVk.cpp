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

#include "DebugVk.h"
#include "PhysicalDeviceVk.h"
#include "SurfaceVk.h"


using namespace Kodiak;
using namespace std;


#if ENABLE_VULKAN_VALIDATION
PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback{ nullptr };
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback{ nullptr };
PFN_vkDebugReportMessageEXT vkDebugReportMessage{ nullptr };
#endif

#if ENABLE_VULKAN_DEBUG_MARKUP
PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag{ nullptr };
PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName{ nullptr };
PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin{ nullptr };
PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd{ nullptr };
PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert{ nullptr };

bool g_debugMarkupAvailable = false;
#endif


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
	constexpr uint32_t version = VK_API_VERSION_1_2;

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName.c_str();
	appInfo.pEngineName = "Kodiak";
	appInfo.apiVersion = version;

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


shared_ptr<DebugReportCallback> Instance::CreateDebugReportCallback(VkDebugReportFlagsEXT flags)
{
	return make_shared<DebugReportCallback>(shared_from_this(), flags);
}


shared_ptr<Surface> Instance::CreateSurface(HINSTANCE hinst, HWND hwnd)
{
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.hinstance = hinst;
	surfaceCreateInfo.hwnd = hwnd;

	return make_shared<Surface>(shared_from_this(), surfaceCreateInfo);
}


void Instance::InitializeDebugMarkup(const shared_ptr<PhysicalDevice>& physicalDevice)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	if (physicalDevice->IsExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		vkDebugMarkerSetObjectTag =
			reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetInstanceProcAddr(m_instance, "vkDebugMarkerSetObjectTagEXT"));

		vkDebugMarkerSetObjectName =
			reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetInstanceProcAddr(m_instance, "vkDebugMarkerSetObjectNameEXT"));

		vkCmdDebugMarkerBegin =
			reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetInstanceProcAddr(m_instance, "vkCmdDebugMarkerBeginEXT"));

		vkCmdDebugMarkerEnd =
			reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetInstanceProcAddr(m_instance, "vkCmdDebugMarkerEndEXT"));

		vkCmdDebugMarkerInsert =
			reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetInstanceProcAddr(m_instance, "vkCmdDebugMarkerInsertEXT"));

		if (vkDebugMarkerSetObjectName)
		{
			g_debugMarkupAvailable = true;
		}
	}
#endif
}


void Instance::Initialize(const VkInstanceCreateInfo& createInfo)
{
	m_version = createInfo.pApplicationInfo->apiVersion;

	auto res = vkCreateInstance(&createInfo, nullptr, &m_instance);
	if (VK_SUCCESS != res)
	{
		Utility::ExitFatal("Could not create Vulkan instance", "Fatal error");
	}

	EnumeratePhysicalDevices();
	m_cachedPhysicalDevices.resize(GetPhysicalDeviceCount());

#if ENABLE_VULKAN_VALIDATION
	vkCreateDebugReportCallback = 
		reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT"));
	vkDestroyDebugReportCallback = 
		reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT"));
	vkDebugReportMessage = 
		reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(m_instance, "vkDebugReportMessageEXT"));
#endif
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