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

#include "DeviceManagerVk.h"

#include "Utility.h"

#include <iostream>
#include <sstream>


using namespace Kodiak;
using namespace std;


namespace
{

VkInstance s_instance{ VK_NULL_HANDLE };
VkDevice s_device{ VK_NULL_HANDLE };
VkPhysicalDevice s_physicalDevice{ VK_NULL_HANDLE };

VkPhysicalDeviceProperties s_physicalDeviceProperties{};
VkPhysicalDeviceFeatures s_physicalDeviceSupportedFeatures{};
VkPhysicalDeviceFeatures s_physicalDeviceEnabledFeatures{};
VkPhysicalDeviceMemoryProperties s_physicalDeviceMemoryProperties{};

// TODO: handle this differently
vector<string> supportedExtensions;
vector<const char*> enabledExtensions;

struct
{
	uint32_t graphics;
	uint32_t compute;
	uint32_t transfer;
} queueFamilyIndices;

vector<VkQueueFamilyProperties> queueFamilyProperties;

} // anonymous namespace


namespace Kodiak
{

VkDevice GetDevice()
{
	return s_device;
}


// Forward declarations
void InitializeDebugging(VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack);
uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags);
bool IsExtensionSupported(const string& name);


void InitializeGraphicsDevice(const string& appName)
{
	// Create Vulkan instance

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = appName.c_str();
	appInfo.pEngineName = "Kodiak";
	appInfo.apiVersion = VK_API_VERSION_1_0;

	vector<const char*> instanceExtensions = { VK_KHR_SURFACE_EXTENSION_NAME };
	instanceExtensions.push_back(VK_KHR_WIN32_SURFACE_EXTENSION_NAME);

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
		"VK_LAYER_LUNARG_standard_validation"
	};

	instanceCreateInfo.enabledLayerCount = validationLayerCount;
	instanceCreateInfo.ppEnabledLayerNames = validationLayerNames;
#endif

	 auto res = vkCreateInstance(&instanceCreateInfo, nullptr, &s_instance);
	 if (VK_SUCCESS != res)
	 {
		 Utility::ExitFatal("Could not create Vulkan instance", "Fatal error");
	 }


#if ENABLE_VULKAN_VALIDATION
	 // Enable validation for debugging

	 // The report flags determine what type of messages for the layers will be displayed
	 // For validating (debugging) an appplication the error and warning bits should suffice
	 VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	 // Additional flags include performance info, loader and layer debug messages, etc.
	 InitializeDebugging(debugReportFlags, VK_NULL_HANDLE);
#endif


	 // Physical device
	 uint32_t gpuCount = 0;
	 // Get number of available physical devices
	 ThrowIfFailed(vkEnumeratePhysicalDevices(s_instance, &gpuCount, nullptr));
	 assert(gpuCount > 0);
	 // Enumerate devices
	 vector<VkPhysicalDevice> physicalDevices(gpuCount);
	 res = vkEnumeratePhysicalDevices(s_instance, &gpuCount, physicalDevices.data());
	 if (res)
	 {
		 Utility::ExitFatal("Could not enumerate physical devices", "Fatal error");
	 }


	 // GPU selection

	 // Select physical device to be used for the Vulkan example
	 // Defaults to the first device [TODO] unless specified in Configure or on commandline
	 uint32_t selectedDevice = 0;

	 s_physicalDevice = physicalDevices[selectedDevice];

	 // Store properties (including limits), features and memory properties of the phyiscal device (so that examples can check against them)
	 vkGetPhysicalDeviceProperties(s_physicalDevice, &s_physicalDeviceProperties);
	 vkGetPhysicalDeviceFeatures(s_physicalDevice, &s_physicalDeviceSupportedFeatures);
	 vkGetPhysicalDeviceMemoryProperties(s_physicalDevice, &s_physicalDeviceMemoryProperties);

	 // TODO: Setup features here

	 // Desired queues need to be requested upon logical device creation
	 // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
	 // requests different queue types

	 std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	 // Queue family properties, used for setting up requested queues upon device creation
	 uint32_t queueFamilyCount;
	 vkGetPhysicalDeviceQueueFamilyProperties(s_physicalDevice, &queueFamilyCount, nullptr);
	 assert(queueFamilyCount > 0);
	 queueFamilyProperties.resize(queueFamilyCount);
	 vkGetPhysicalDeviceQueueFamilyProperties(s_physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

	 // Get list of supported extensions
	 uint32_t extCount = 0;
	 vkEnumerateDeviceExtensionProperties(s_physicalDevice, nullptr, &extCount, nullptr);
	 if (extCount > 0)
	 {
		 vector<VkExtensionProperties> extensions(extCount);
		 if (vkEnumerateDeviceExtensionProperties(s_physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
		 {
			 for (auto ext : extensions)
			 {
				 supportedExtensions.push_back(ext.extensionName);
			 }
		 }
	 }

	 // Get queue family indices for the requested queue family types
	 // Note that the indices may overlap depending on the implementation

	 const float defaultQueuePriority = 0.0f;

	 // Graphics queue
	 queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
	 VkDeviceQueueCreateInfo queueInfo{};
	 queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	 queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
	 queueInfo.queueCount = 1;
	 queueInfo.pQueuePriorities = &defaultQueuePriority;
	 queueCreateInfos.push_back(queueInfo);

	 // Dedicated compute queue
	 queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
	 if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
	 {
		 // If compute family index differs, we need an additional queue create info for the compute queue
		 VkDeviceQueueCreateInfo queueInfo{};
		 queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		 queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
		 queueInfo.queueCount = 1;
		 queueInfo.pQueuePriorities = &defaultQueuePriority;
		 queueCreateInfos.push_back(queueInfo);
	 }

	 // Dedicated transfer queue
	queueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
	if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer != queueFamilyIndices.compute))
	{
		// If compute family index differs, we need an additional queue create info for the compute queue
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueInfo);
	}

	// Create the logical device representation
	vector<const char*> deviceExtensions(enabledExtensions);
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &s_physicalDeviceEnabledFeatures;

	// Enable the debug marker extension if it is present (likely meaning a debugging tool is present)
	if (IsExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
	{
		deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		//enableDebugMarkers = true;
	}

	if (deviceExtensions.size() > 0)
	{
		deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
	}

	VkResult result = vkCreateDevice(s_physicalDevice, &deviceCreateInfo, nullptr, &s_device);
}


void ShutdownGraphicsDevice()
{
	vkDestroyDevice(s_device, nullptr);
	s_device = VK_NULL_HANDLE;

	s_physicalDevice = VK_NULL_HANDLE;

	vkDestroyInstance(s_instance, nullptr);
	s_instance = VK_NULL_HANDLE;
}


PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = VK_NULL_HANDLE;
PFN_vkDebugReportMessageEXT dbgBreakCallback = VK_NULL_HANDLE;

VkDebugReportCallbackEXT msgCallback;

VkBool32 messageCallback(
	VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t srcObject,
	size_t location,
	int32_t msgCode,
	const char* pLayerPrefix,
	const char* pMsg,
	void* pUserData)
{
	// Select prefix depending on flags passed to the callback
	// Note that multiple flags may be set for a single validation message
	string prefix("");

	// Error that may result in undefined behaviour
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		prefix += "ERROR:";
	};
	// Warnings may hint at unexpected / non-spec API usage
	if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT)
	{
		prefix += "WARNING:";
	};
	// May indicate sub-optimal usage of the API
	if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT)
	{
		prefix += "PERFORMANCE:";
	};
	// Informal messages that may become handy during debugging
	if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT)
	{
		prefix += "INFO:";
	}
	// Diagnostic info from the Vulkan loader and layers
	// Usually not helpful in terms of API usage, but may help to debug layer and loader problems 
	if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT)
	{
		prefix += "DEBUG:";
	}

	// Display message to default output (console/logcat)
	stringstream debugMessage;
	debugMessage << prefix << " [" << pLayerPrefix << "] Code " << msgCode << " : " << pMsg;

	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
		cerr << debugMessage.str() << "\n";
	}
	else {
		cout << debugMessage.str() << "\n";
	}

	fflush(stdout);

	// The return value of this callback controls wether the Vulkan call that caused
	// the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
	// (and return a VkResult) to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will 
	// return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}


void InitializeDebugging(VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack)
{
	CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(s_instance, "vkCreateDebugReportCallbackEXT"));
	DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(s_instance, "vkDestroyDebugReportCallbackEXT"));
	dbgBreakCallback = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(s_instance, "vkDebugReportMessageEXT"));

	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
	dbgCreateInfo.flags = flags;

	VkResult err = CreateDebugReportCallback(
		s_instance,
		&dbgCreateInfo,
		nullptr,
		(callBack != VK_NULL_HANDLE) ? &callBack : &msgCallback);
	assert(!err);
}


uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
{
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


bool IsExtensionSupported(const string& name)
{
	return find(begin(supportedExtensions), end(supportedExtensions), name) != end(supportedExtensions);
}

} // namespace Kodiak