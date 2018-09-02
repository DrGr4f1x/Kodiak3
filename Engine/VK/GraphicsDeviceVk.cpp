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

#include "GraphicsDevice.h"

#include "Utility.h"

#include "CommandContextVk.h"
#include "CommandListManagerVk.h"
#include "PipelineStateVk.h"
#include "RootSignatureVk.h"
#include "TextureVk.h"

#include <iostream>
#include <sstream>


using namespace Kodiak;
using namespace std;


namespace
{
PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallback = VK_NULL_HANDLE;
PFN_vkDestroyDebugReportCallbackEXT DestroyDebugReportCallback = VK_NULL_HANDLE;
PFN_vkDebugReportMessageEXT dbgBreakCallback = VK_NULL_HANDLE;

VkDebugReportCallbackEXT msgCallback = VK_NULL_HANDLE;
} // anonymous namespace

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

	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		cerr << debugMessage.str() << "\n";
	}
	else
	{
		cout << debugMessage.str() << "\n";
	}

	fflush(stdout);

	// The return value of this callback controls whether the Vulkan call that caused
	// the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
	// (and return a VkResult) to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will 
	// return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}


namespace
{

DeviceHandle g_device{ nullptr };

Format BackBufferColorFormat = Format::R10G10B10A2_UNorm;
Format DepthFormat = Format::D32_Float_S8_UInt;

#if ENABLE_VULKAN_DEBUG_MARKUP
PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag{ nullptr };
PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName{ nullptr };
PFN_vkCmdDebugMarkerBeginEXT vkCmdDebugMarkerBegin{ nullptr };
PFN_vkCmdDebugMarkerEndEXT vkCmdDebugMarkerEnd{ nullptr };
PFN_vkCmdDebugMarkerInsertEXT vkCmdDebugMarkerInsert{ nullptr };

bool g_debugMarkupAvailable = false;

void SetDebugName(uint64_t obj, VkDebugReportObjectTypeEXT objType, const char* name)
{
	if (g_debugMarkupAvailable)
	{
		VkDebugMarkerObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_MARKER_OBJECT_NAME_INFO_EXT;
		nameInfo.pNext = nullptr;
		nameInfo.objectType = objType;
		nameInfo.object = obj;
		nameInfo.pObjectName = name;
		vkDebugMarkerSetObjectName(g_device, &nameInfo);
	}
}
#endif


InstanceHandle CreateInstance(const string& appName)
{
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

	VkInstance instance = VK_NULL_HANDLE;
	auto res = vkCreateInstance(&instanceCreateInfo, nullptr, &instance);
	if (VK_SUCCESS != res)
	{
		Utility::ExitFatal("Could not create Vulkan instance", "Fatal error");
	}

	return InstanceHandle::Create(instance);
}


void InitializeDebugging(const InstanceHandle& instance, VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack)
{
	CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugReportCallbackEXT"));
	DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugReportCallbackEXT"));
	dbgBreakCallback = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(instance, "vkDebugReportMessageEXT"));

	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
	dbgCreateInfo.flags = flags;

	VkResult err = CreateDebugReportCallback(
		instance,
		&dbgCreateInfo,
		nullptr,
		(callBack != VK_NULL_HANDLE) ? &callBack : &msgCallback);
	assert(!err);
}


void FreeDebugCallback(VkInstance instance)
{
	if (msgCallback != VK_NULL_HANDLE)
	{
		DestroyDebugReportCallback(instance, msgCallback, nullptr);
	}
	msgCallback = VK_NULL_HANDLE;
}


void FindBestDepthFormat(VkPhysicalDevice physicalDevice)
{
	// Since all depth formats may be optional, we need to find a suitable depth format to use
	// Start with the highest precision packed format
	vector<VkFormat> depthFormats =
	{
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		VK_FORMAT_D32_SFLOAT,
		VK_FORMAT_D24_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM_S8_UINT,
		VK_FORMAT_D16_UNORM
	};

	for (auto& format : depthFormats)
	{
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProps);
		// Format must support depth stencil attachment for optimal tiling
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			DepthFormat = MapVulkanFormatToEngine(format);
			return;
		}
	}
}

} // anonymous namespace


namespace Kodiak
{

struct GraphicsDevice::PlatformData : public NonCopyable
{
	PlatformData() = default;
	~PlatformData() { Destroy(); }

	void Destroy()
	{
		g_device = nullptr;

		for (int i = 0; i < NumSwapChainBuffers; ++i)
		{
			vkDestroyFence(device, presentFences[i], nullptr);
			presentFences[i] = VK_NULL_HANDLE;
		}

		vkDestroySurfaceKHR(instance, surface, nullptr);
		surface = VK_NULL_HANDLE;

		vkDestroySwapchainKHR(device, swapChain, nullptr);
		swapChain = VK_NULL_HANDLE;

		FreeDebugCallback(instance);

		device = nullptr;
		instance = nullptr;
	}

	void SelectPhysicalDevice()
	{
		uint32_t gpuCount = 0;
		// Get number of available physical devices
		ThrowIfFailed(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
		assert(gpuCount > 0);
		// Enumerate devices
		vector<VkPhysicalDevice> physicalDevices(gpuCount);
		auto res = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
		if (res)
		{
			Utility::ExitFatal("Could not enumerate physical devices", "Fatal error");
		}

		// GPU selection

		physicalDevice = physicalDevices[0];

		// Store properties (including limits), features and memory properties of the physical device (so that examples can check against them)
		vkGetPhysicalDeviceProperties(physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceFeatures(physicalDevice, &physicalDeviceSupportedFeatures);
		vkGetPhysicalDeviceMemoryProperties(physicalDevice, &physicalDeviceMemoryProperties);

		if (physicalDeviceSupportedFeatures.textureCompressionBC == VK_TRUE)
		{
			physicalDeviceEnabledFeatures.textureCompressionBC = VK_TRUE;
		}

		if (physicalDeviceSupportedFeatures.tessellationShader == VK_TRUE)
		{
			physicalDeviceEnabledFeatures.tessellationShader = VK_TRUE;
		}

		if (physicalDeviceSupportedFeatures.geometryShader == VK_TRUE)
		{
			physicalDeviceEnabledFeatures.geometryShader = VK_TRUE;
		}

		if (physicalDeviceSupportedFeatures.fillModeNonSolid == VK_TRUE)
		{
			physicalDeviceEnabledFeatures.fillModeNonSolid = VK_TRUE;
		}
	}

	void InitializeDebugMarkup()
	{
#if ENABLE_VULKAN_DEBUG_MARKUP
		bool extensionPresent = false;

		// Check if the debug marker extension is present (which is the case if run from a graphics debugger)
		uint32_t extensionCount;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);
		std::vector<VkExtensionProperties> extensions(extensionCount);
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensions.data());
		for (auto extension : extensions)
		{
			if (strcmp(extension.extensionName, VK_EXT_DEBUG_MARKER_EXTENSION_NAME) == 0)
			{
				extensionPresent = true;
				break;
			}
		}

		if (extensionPresent)
		{
			vkDebugMarkerSetObjectTag =
				reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetInstanceProcAddr(instance, "vkDebugMarkerSetObjectTagEXT"));

			vkDebugMarkerSetObjectName =
				reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetInstanceProcAddr(instance, "vkDebugMarkerSetObjectNameEXT"));

			vkCmdDebugMarkerBegin =
				reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetInstanceProcAddr(instance, "vkCmdDebugMarkerBeginEXT"));

			vkCmdDebugMarkerEnd =
				reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetInstanceProcAddr(instance, "vkCmdDebugMarkerEndEXT"));

			vkCmdDebugMarkerInsert =
				reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetInstanceProcAddr(instance, "vkCmdDebugMarkerInsertEXT"));

			if (vkDebugMarkerSetObjectName)
			{
				g_debugMarkupAvailable = true;
			}
		}
#endif
	}

	void CreateLogicalDevice()
	{
		// Desired queues need to be requested upon logical device creation
		// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
		// requests different queue types

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

		// Queue family properties, used for setting up requested queues upon device creation
		uint32_t queueFamilyCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
		assert(queueFamilyCount > 0);
		queueFamilyProperties.resize(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

		// Get list of supported extensions
		uint32_t extCount = 0;
		vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
		if (extCount > 0)
		{
			vector<VkExtensionProperties> extensions(extCount);
			if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
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
			// If compute family index differs, we need an additional queue create info for the transfer queue
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
		deviceExtensions.push_back(VK_KHR_MAINTENANCE1_EXTENSION_NAME);
		//deviceExtensions.push_back(VK_KHR_MAINTENANCE2_EXTENSION_NAME);

		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
		deviceCreateInfo.pEnabledFeatures = &physicalDeviceEnabledFeatures;

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

		VkDevice logicalDevice{ VK_NULL_HANDLE };
		VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
		ThrowIfFailed(result);

		device = DeviceHandle::Create(logicalDevice);

		// Get a graphics queue from the device
		vkGetDeviceQueue(device, queueFamilyIndices.graphics, 0, &graphicsQueue);
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

	bool IsExtensionSupported(const string& name) const
	{
		return find(begin(supportedExtensions), end(supportedExtensions), name) != end(supportedExtensions);
	}

	void InitSurface(HINSTANCE hInstance, HWND hWnd)
	{
		VkResult err = VK_SUCCESS;

		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = hInstance;
		surfaceCreateInfo.hwnd = hWnd;
		err = vkCreateWin32SurfaceKHR(instance, &surfaceCreateInfo, nullptr, &surface);

		if (err != VK_SUCCESS)
		{
			Utility::ExitFatal("Could not create surface!", "Fatal error");
		}

		// Get available queue family properties
		uint32_t queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
		assert(queueCount >= 1);

		vector<VkQueueFamilyProperties> queueProps(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, queueProps.data());

		// Iterate over each queue to learn whether it supports presenting:
		// Find a queue with present support
		// Will be used to present the swap chain images to the windowing system
		vector<VkBool32> supportsPresent(queueCount);
		for (uint32_t i = 0; i < queueCount; i++)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &supportsPresent[i]);
		}

		// Search for a graphics and a present queue in the array of queue
		// families, try to find one that supports both
		uint32_t graphicsQueueNodeIndex = UINT32_MAX;
		uint32_t presentQueueNodeIndex = UINT32_MAX;
		for (uint32_t i = 0; i < queueCount; i++)
		{
			if ((queueProps[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (graphicsQueueNodeIndex == UINT32_MAX)
				{
					graphicsQueueNodeIndex = i;
				}

				if (supportsPresent[i] == VK_TRUE)
				{
					graphicsQueueNodeIndex = i;
					presentQueueNodeIndex = i;
					break;
				}
			}
		}
		if (presentQueueNodeIndex == UINT32_MAX)
		{
			// If there's no queue that supports both present and graphics
			// try to find a separate present queue
			for (uint32_t i = 0; i < queueCount; ++i)
			{
				if (supportsPresent[i] == VK_TRUE)
				{
					presentQueueNodeIndex = i;
					break;
				}
			}
		}

		// Exit if either a graphics or a presenting queue hasn't been found
		if (graphicsQueueNodeIndex == UINT32_MAX || presentQueueNodeIndex == UINT32_MAX)
		{
			Utility::ExitFatal("Could not find a graphics and/or presenting queue!", "Fatal error");
		}

		// todo : Add support for separate graphics and presenting queue
		if (graphicsQueueNodeIndex != presentQueueNodeIndex)
		{
			Utility::ExitFatal("Separate graphics and presenting queues are not supported yet!", "Fatal error");
		}

		presentQueueNodeIndex = graphicsQueueNodeIndex;

		// Get list of supported surface formats
		uint32_t formatCount;
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, nullptr));
		assert(formatCount > 0);

		vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &formatCount, surfaceFormats.data()));

		// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
		// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
		if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
		{
			BackBufferColorFormat = MapVulkanFormatToEngine(VK_FORMAT_B8G8R8A8_UNORM);
			colorSpace = surfaceFormats[0].colorSpace;
		}
		else
		{
			// iterate over the list of available surface format and
			// check for the presence of VK_FORMAT_B8G8R8A8_UNORM
			bool found_B8G8R8A8_UNORM = false;
			for (auto&& surfaceFormat : surfaceFormats)
			{
				if (surfaceFormat.format == VK_FORMAT_B8G8R8A8_UNORM)
				{
					BackBufferColorFormat = MapVulkanFormatToEngine(surfaceFormat.format);
					colorSpace = surfaceFormat.colorSpace;
					found_B8G8R8A8_UNORM = true;
					break;
				}
			}

			// in case VK_FORMAT_B8G8R8A8_UNORM is not available
			// select the first available color format
			if (!found_B8G8R8A8_UNORM)
			{
				BackBufferColorFormat = MapVulkanFormatToEngine(surfaceFormats[0].format);
				colorSpace = surfaceFormats[0].colorSpace;
			}
		}
	}

	void CreateSwapChain(uint32_t* width, uint32_t* height, bool vsync)
	{
		VkSwapchainKHR oldSwapchain = swapChain;

		// Get physical device surface properties and formats
		VkSurfaceCapabilitiesKHR surfCaps;
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfCaps));

		// Get available present modes
		uint32_t presentModeCount;
		ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr));
		assert(presentModeCount > 0);

		vector<VkPresentModeKHR> presentModes(presentModeCount);
		ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes.data()));

		VkExtent2D swapchainExtent = {};
		// If width (and height) equals the special value 0xFFFFFFFF, the size of the surface will be set by the swapchain
		if (surfCaps.currentExtent.width == (uint32_t)-1)
		{
			// If the surface size is undefined, the size is set to
			// the size of the images requested.
			swapchainExtent.width = *width;
			swapchainExtent.height = *height;
		}
		else
		{
			// If the surface size is defined, the swap chain size must match
			swapchainExtent = surfCaps.currentExtent;
			*width = surfCaps.currentExtent.width;
			*height = surfCaps.currentExtent.height;
		}

		// Select a present mode for the swapchain

		// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
		// This mode waits for the vertical blank ("v-sync")
		VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

		// If v-sync is not requested, try to find a mailbox mode
		// It's the lowest latency non-tearing present mode available
		if (!vsync)
		{
			for (size_t i = 0; i < presentModeCount; ++i)
			{
				if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
				{
					swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
					break;
				}
				if ((swapchainPresentMode != VK_PRESENT_MODE_MAILBOX_KHR) && (presentModes[i] == VK_PRESENT_MODE_IMMEDIATE_KHR))
				{
					swapchainPresentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
				}
			}
		}

		// Determine the number of images
		uint32_t desiredNumberOfSwapchainImages = surfCaps.minImageCount + 1;
		if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
		{
			desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
		}

		// Find the transformation of the surface
		VkSurfaceTransformFlagsKHR preTransform;
		if (surfCaps.supportedTransforms & VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			// We prefer a non-rotated transform
			preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		}
		else
		{
			preTransform = surfCaps.currentTransform;
		}

		// Find a supported composite alpha format (not all devices support alpha opaque)
		VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		// Simply select the first composite alpha format available
		vector<VkCompositeAlphaFlagBitsKHR> compositeAlphaFlags =
		{
			VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
			VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR,
			VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,
		};

		for (auto& compositeAlphaFlag : compositeAlphaFlags)
		{
			if (surfCaps.supportedCompositeAlpha & compositeAlphaFlag)
			{
				compositeAlpha = compositeAlphaFlag;
				break;
			};
		}

		auto vkFormat = static_cast<VkFormat>(BackBufferColorFormat);

		VkSwapchainCreateInfoKHR swapchainCI = {};
		swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCI.pNext = nullptr;
		swapchainCI.surface = surface;
		swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
		swapchainCI.imageFormat = vkFormat;
		swapchainCI.imageColorSpace = colorSpace;
		swapchainCI.imageExtent = { swapchainExtent.width, swapchainExtent.height };
		swapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		swapchainCI.preTransform = (VkSurfaceTransformFlagBitsKHR)preTransform;
		swapchainCI.imageArrayLayers = 1;
		swapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCI.queueFamilyIndexCount = 0;
		swapchainCI.pQueueFamilyIndices = nullptr;
		swapchainCI.presentMode = swapchainPresentMode;
		swapchainCI.oldSwapchain = oldSwapchain;
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		swapchainCI.clipped = VK_TRUE;
		swapchainCI.compositeAlpha = compositeAlpha;

		// Set additional usage flag for blitting from the swapchain images if supported
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(physicalDevice, vkFormat, &formatProps);
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)
		{
			swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		ThrowIfFailed(vkCreateSwapchainKHR(device, &swapchainCI, nullptr, &swapChain));

		// If an existing swap chain is re-created, destroy the old swap chain
		// This also cleans up all the presentable images
		if (oldSwapchain != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device, oldSwapchain, nullptr);
		}

		uint32_t imageCount{ 0 };
		ThrowIfFailed(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr));

		assert(imageCount == NumSwapChainBuffers);

		// Get the swap chain images
		ThrowIfFailed(vkGetSwapchainImagesKHR(device, swapChain, &imageCount, images.data()));
	}

	void CreateFences()
	{
		for (int i = 0; i < NumSwapChainBuffers; ++i)
		{
			VkFenceCreateInfo fenceInfo = { VK_STRUCTURE_TYPE_FENCE_CREATE_INFO };
			fenceInfo.pNext = nullptr;
			fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
			
			vkCreateFence(device, &fenceInfo, nullptr, &presentFences[i]);
		}
	}

	InstanceHandle instance;

	VkPhysicalDevice physicalDevice{ VK_NULL_HANDLE };
	VkPhysicalDeviceProperties physicalDeviceProperties{};
	VkPhysicalDeviceFeatures physicalDeviceSupportedFeatures{};
	VkPhysicalDeviceFeatures physicalDeviceEnabledFeatures{};
	VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties{};

	DeviceHandle device;

	struct
	{
		uint32_t graphics;
		uint32_t compute;
		uint32_t transfer;
	} queueFamilyIndices;

	vector<VkQueueFamilyProperties> queueFamilyProperties;
	VkQueue graphicsQueue{ VK_NULL_HANDLE };

	// TODO: handle this differently
	vector<string> supportedExtensions;
	vector<const char*> enabledExtensions;

	VkSurfaceKHR surface{ VK_NULL_HANDLE };
	VkSwapchainKHR swapChain{ VK_NULL_HANDLE };
	uint32_t presentQueueNodeIndex{ 0 };
	VkColorSpaceKHR colorSpace;

	array<VkImage, NumSwapChainBuffers> images;
	
	// Present and flip fences
	array<VkFence, NumSwapChainBuffers> presentFences;
};

} // namespace Kodiak


GraphicsDevice::~GraphicsDevice() = default;


Format GraphicsDevice::GetColorFormat() const
{
	return BackBufferColorFormat;
}


Format GraphicsDevice::GetDepthFormat() const
{
	return DepthFormat;
}


uint32_t GraphicsDevice::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
{
	for (uint32_t i = 0; i < m_platformData->physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((m_platformData->physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
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


void GraphicsDevice::PlatformCreate()
{
	m_platformData = new PlatformData;

	m_platformData->instance = CreateInstance(m_appName);

#if ENABLE_VULKAN_VALIDATION
	// Enable validation for debugging

	// The report flags determine what type of messages for the layers will be displayed
	// For validating (debugging) an application the error and warning bits should suffice
	VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	// Additional flags include performance info, loader and layer debug messages, etc.
	InitializeDebugging(m_platformData->instance, debugReportFlags, VK_NULL_HANDLE);
#endif

	m_platformData->SelectPhysicalDevice();

#if ENABLE_VULKAN_DEBUG_MARKUP
	m_platformData->InitializeDebugMarkup();
#endif

	// TODO: Setup features here

	m_platformData->CreateLogicalDevice();

	g_device = m_platformData->device;

	m_platformData->InitSurface(m_hinst, m_hwnd);
	m_platformData->CreateSwapChain(&m_width, &m_height, false /* vsync */);

	m_platformData->CreateFences();

	// Get ColorBuffers for swap chain images
	for (uint32_t i = 0; i < NumSwapChainBuffers; ++i)
	{
		m_swapChainBuffers[i] = make_shared<ColorBuffer>();
		m_swapChainBuffers[i]->CreateFromSwapChain("Primary SwapChain Buffer", m_platformData->images[i], m_width, m_height, BackBufferColorFormat);
	}

	g_commandManager.Create(
		m_platformData->queueFamilyIndices.graphics, m_platformData->graphicsQueue,
		m_platformData->queueFamilyIndices.compute, m_platformData->graphicsQueue,
		m_platformData->queueFamilyIndices.transfer, m_platformData->graphicsQueue);
}


void GraphicsDevice::PlatformPresent()
{
	// Present
	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &m_platformData->swapChain;
	presentInfo.pImageIndices = &m_currentBuffer;
	vkQueuePresentKHR(m_platformData->graphicsQueue, &presentInfo);

	// Flip
	VkDevice device = m_platformData->device;

	vkWaitForFences(device, 1, &m_platformData->presentFences[m_currentBuffer], false, UINT64_MAX);
	uint32_t nextBuffer = (m_currentBuffer + 1) % NumSwapChainBuffers;

	VkFence fence = m_platformData->presentFences[nextBuffer];
	vkResetFences(device, 1, &fence);

	vkAcquireNextImageKHR(device, m_platformData->swapChain, UINT64_MAX, nullptr, fence, &m_currentBuffer);
}


void GraphicsDevice::PlatformDestroyData()
{
	delete m_platformData;
	m_platformData = nullptr;
}


void GraphicsDevice::PlatformDestroy()
{}


VkFormatProperties GraphicsDevice::GetFormatProperties(Format format)
{
	VkFormat vkFormat = static_cast<VkFormat>(format);
	VkFormatProperties properties{};

	vkGetPhysicalDeviceFormatProperties(m_platformData->physicalDevice, vkFormat, &properties);

	return properties;
}


void GraphicsDevice::WaitForGpuIdle()
{
	g_commandManager.IdleGPU();
}


const DeviceHandle& Kodiak::GetDevice()
{
	return g_device;
}


VkFormatProperties Kodiak::GetFormatProperties(Format format)
{
	return g_graphicsDevice->GetFormatProperties(format);
}


uint32_t Kodiak::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
{
	return g_graphicsDevice->GetMemoryTypeIndex(typeBits, properties, memTypeFound);
}


// TODO
#if 0
void Kodiak::SetDebugName(VkInstance obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkPhysicalDevice obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkDevice obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkQueue obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkSemaphore obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkCommandBuffer obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkFence obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkDeviceMemory obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkBuffer obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkImage obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkEvent obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkQueryPool obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkBufferView obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkImageView obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkShaderModule obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkPipelineCache obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkPipelineLayout obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkRenderPass obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkPipeline obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkDescriptorSetLayout obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkSampler obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkDescriptorPool obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkDescriptorSet obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkFramebuffer obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkCommandPool obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkSurfaceKHR obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkSwapchainKHR obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, name.c_str());
#endif
}


void Kodiak::SetDebugName(VkDebugReportCallbackEXT obj, const std::string& name)
{
#if ENABLE_VULKAN_DEBUG_MARKUP
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT, name.c_str());
#endif
}
#endif