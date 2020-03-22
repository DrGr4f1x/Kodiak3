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

#include "GraphicsFeatures.h"
#include "PipelineState.h"
#include "Texture.h"
#include "Utility.h"

#include "CommandContextVk.h"
#include "CommandListManagerVk.h"
#include "DescriptorHeapVk.h"
#include "RootSignatureVk.h"

#include <iostream>
#include <sstream>

#define VMA_IMPLEMENTATION
#include "Extern\VulkanMemoryAllocator\vk_mem_alloc.h"

using namespace Kodiak;
using namespace Utility;
using namespace std;


#if ENABLE_VULKAN_VALIDATION
PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback{ nullptr };
PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback{ nullptr };
PFN_vkDebugReportMessageEXT vkDebugReportMessage{ nullptr };

VkDebugReportCallbackEXT msgCallback = VK_NULL_HANDLE;

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

	// Error that may result in undefined behavior
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

	OutputDebugString(debugMessage.str().c_str());
	if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT)
	{
		if (strstr(pMsg, "VUID-vkDestroyBuffer-buffer-00922") == nullptr &&
			strstr(pMsg, "VUID-vkFreeMemory-memory-00677") == nullptr &&
			strstr(pMsg, "VUID-vkResetCommandBuffer-commandBuffer-00045") == nullptr)
		{
			assert(false);
		}
	}

	fflush(stderr);

	// The return value of this callback controls whether the Vulkan call that caused
	// the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
	// (and return a VkResult) to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will 
	// return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}
#endif

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
		vkDebugMarkerSetObjectName(GetDevice(), &nameInfo);
	}
}
#endif


namespace
{

// TODO - Delete me
shared_ptr<DeviceRef> g_device;

Format BackBufferColorFormat = Format::R10G10B10A2_UNorm;
Format DepthFormat = Format::D32_Float_S8_UInt;


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
	PlatformData()
	{
		supportedDeviceFeatures2.pNext = &supportedDeviceFeatures1_2;
		enabledDeviceFeatures2.pNext = &enabledDeviceFeatures1_2;
	}


	void CreateInstance(const string& appName)
	{
		VkApplicationInfo appInfo = {};
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pApplicationName = appName.c_str();
		appInfo.pEngineName = "Kodiak";
		appInfo.apiVersion = VK_API_VERSION_1_2;

		const vector<const char*> instanceExtensions =
		{
#if ENABLE_VULKAN_VALIDATION
			VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		};

		const vector<const char*> instanceLayers =
		{
#if ENABLE_VULKAN_VALIDATION
			"VK_LAYER_KHRONOS_validation"
#endif
		};

		VkInstanceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.pApplicationInfo = &appInfo;
		createInfo.enabledExtensionCount = (uint32_t)instanceExtensions.size();
		createInfo.ppEnabledExtensionNames = instanceExtensions.data();
		createInfo.enabledLayerCount = (uint32_t)instanceLayers.size();
		createInfo.ppEnabledLayerNames = instanceLayers.data();

		VkInstance vkInstance{ VK_NULL_HANDLE };
		auto res = vkCreateInstance(&createInfo, nullptr, &vkInstance);
		if (VK_SUCCESS != res)
		{
			Utility::ExitFatal("Could not create Vulkan instance", "Fatal error");
		}

		instance = InstanceRef::Create(vkInstance);
	}
	

	void EnumeratePhysicalDevices()
	{
		uint32_t gpuCount = 0;
		// Get number of available physical devices
		ThrowIfFailed(vkEnumeratePhysicalDevices(*instance, &gpuCount, nullptr));
		assert(gpuCount > 0);

		// Enumerate devices
		physicalDevices.resize(gpuCount);
		auto res = vkEnumeratePhysicalDevices(*instance, &gpuCount, physicalDevices.data());
		if (res)
		{
			Utility::ExitFatal("Could not enumerate physical devices", "Fatal error");
		}
	}


	void InitializeDebugMarkup()
	{
#if ENABLE_VULKAN_DEBUG_MARKUP
		if (IsExtensionSupported(VK_EXT_DEBUG_MARKER_EXTENSION_NAME))
		{
			vkDebugMarkerSetObjectTag =
				reinterpret_cast<PFN_vkDebugMarkerSetObjectTagEXT>(vkGetInstanceProcAddr(GetInstance(), "vkDebugMarkerSetObjectTagEXT"));

			vkDebugMarkerSetObjectName =
				reinterpret_cast<PFN_vkDebugMarkerSetObjectNameEXT>(vkGetInstanceProcAddr(GetInstance(), "vkDebugMarkerSetObjectNameEXT"));

			vkCmdDebugMarkerBegin =
				reinterpret_cast<PFN_vkCmdDebugMarkerBeginEXT>(vkGetInstanceProcAddr(GetInstance(), "vkCmdDebugMarkerBeginEXT"));

			vkCmdDebugMarkerEnd =
				reinterpret_cast<PFN_vkCmdDebugMarkerEndEXT>(vkGetInstanceProcAddr(GetInstance(), "vkCmdDebugMarkerEndEXT"));

			vkCmdDebugMarkerInsert =
				reinterpret_cast<PFN_vkCmdDebugMarkerInsertEXT>(vkGetInstanceProcAddr(GetInstance(), "vkCmdDebugMarkerInsertEXT"));

			if (vkDebugMarkerSetObjectName)
			{
				g_debugMarkupAvailable = true;
			}
		}
#endif
	}


	void InitializeValidation()
	{
#if ENABLE_VULKAN_VALIDATION
		vkCreateDebugReportCallback =
			reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(GetInstance(), "vkCreateDebugReportCallbackEXT"));
		vkDestroyDebugReportCallback =
			reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(GetInstance(), "vkDestroyDebugReportCallbackEXT"));
		vkDebugReportMessage =
			reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(GetInstance(), "vkDebugReportMessageEXT"));

		assert(vkCreateDebugReportCallback);
		assert(vkDestroyDebugReportCallback);

		// The report flags determine what type of messages for the layers will be displayed
		// For validating (debugging) an application the error and warning bits should suffice
		VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;

		VkDebugReportCallbackCreateInfoEXT createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
		createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
		createInfo.flags = debugReportFlags;

		ThrowIfFailed(vkCreateDebugReportCallback(GetInstance(), &createInfo, nullptr, &msgCallback));
#endif
	}


	void GetPhysicalDeviceProperties()
	{
		// Get device and memory properties
		vkGetPhysicalDeviceProperties(*physicalDevice, &physicalDeviceProperties);
		vkGetPhysicalDeviceMemoryProperties(*physicalDevice, &physicalDeviceMemoryProperties);

		// Get device features
		vkGetPhysicalDeviceFeatures2(*physicalDevice, &supportedDeviceFeatures2);

		// Get list of supported extensions
		uint32_t extCount = 0;
		ThrowIfFailed(vkEnumerateDeviceExtensionProperties(*physicalDevice, nullptr, &extCount, nullptr));
		if (extCount > 0)
		{
			deviceExtensions.resize(extCount);
			ThrowIfFailed(vkEnumerateDeviceExtensionProperties(*physicalDevice, nullptr, &extCount, &deviceExtensions.front()));
		}

		// Get available queue family properties
		uint32_t queueCount;
		vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &queueCount, nullptr);
		assert(queueCount >= 1);

		queueFamilyProperties.resize(queueCount);
		vkGetPhysicalDeviceQueueFamilyProperties(*physicalDevice, &queueCount, queueFamilyProperties.data());
	}


	bool GetSurfaceSupport(uint32_t index, VkSurfaceKHR surface) const
	{
		VkBool32 supported = VK_FALSE;
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceSupportKHR(*physicalDevice, index, surface, &supported));
		return supported == VK_TRUE;
	}


	vector<uint32_t> GetGraphicsPresentQueueFamilyIndices(VkSurfaceKHR surface)
	{
		vector<uint32_t> indices;
		for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
		{
			if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && GetSurfaceSupport((uint32_t)i, surface))
			{
				indices.push_back((uint32_t)i);
			}
		}
		return indices;
	}


	vector<uint32_t> GetQueueFamilyIndices(VkQueueFlags queueFlags)
	{
		vector<uint32_t> indices;
		for (size_t i = 0; i < queueFamilyProperties.size(); ++i)
		{
			if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
			{
				indices.push_back((uint32_t)i);
			}
		}
		return indices;
	}


	uint32_t GetQueueFamilyIndex(VkQueueFlags queueFlags)
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


	uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
	{
		for (uint32_t i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++)
		{
			if ((typeBits & 1) == 1)
			{
				if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
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


	void SelectPhysicalDevice()
	{
		// GPU selection
		EnumeratePhysicalDevices();
		physicalDevice = PhysicalDeviceRef::Create(instance, physicalDevices[0]);

		// Initialize debug markup and validation callbacks
		InitializeDebugMarkup();
		InitializeValidation();

		// Get available physical device properties and features
		GetPhysicalDeviceProperties();

		// Record which extensions are required or optional, based on Application config
		GatherApplicationExtensions(false);
		GatherApplicationExtensions(true);
		ValidateApplicationExtensions();

		// Enabled required and optional features, as requested by the application
		EnableFeatures(false);
		EnableFeatures(true);

		// Report missing features and exit
		if (!unsupportedRequiredFeatures.empty())
		{
			string errMsg;
			string errDetails;
			if (unsupportedRequiredFeatures.size() > 1)
			{
				errMsg = "Required Features Not Supported";
				errDetails = "This Application requires:\n ";
				for (size_t i = 0; i < unsupportedRequiredFeatures.size(); ++i)
					errDetails += unsupportedRequiredFeatures[i] + "\n";
				errDetails += "\n, which are unavailable.  You may need to update your GPU or graphics driver";
			}
			else
			{
				errMsg = "Required Feature Not Supported";
				errDetails = "This Application requires:\n " + unsupportedRequiredFeatures[0] + "\n, which is unavailable.  You may need to update your GPU or graphics driver";

			}
			ExitFatal(errMsg, errDetails);
		}
	}


	void CreateSemaphores()
	{
		VkSemaphoreTypeCreateInfo typeCreateInfo;
		typeCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO;
		typeCreateInfo.pNext = nullptr;
		typeCreateInfo.semaphoreType = VK_SEMAPHORE_TYPE_BINARY;
		typeCreateInfo.initialValue = 0;

		VkSemaphoreCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		createInfo.pNext = &typeCreateInfo;
		createInfo.flags = 0;

		// imageAcquireSemaphore
		{
			VkSemaphore vkSemaphore{ VK_NULL_HANDLE };
			ThrowIfFailed(vkCreateSemaphore(*device, &createInfo, nullptr, &vkSemaphore));
			imageAcquireSemaphore = SemaphoreRef::Create(device, vkSemaphore);
		}

		// presentSemaphore
		{
			VkSemaphore vkSemaphore{ VK_NULL_HANDLE };
			ThrowIfFailed(vkCreateSemaphore(*device, &createInfo, nullptr, &vkSemaphore));
			presentSemaphore = SemaphoreRef::Create(device, vkSemaphore);
		}
	}


	void CreateLogicalDevice()
	{
		// Desired queues need to be requested upon logical device creation
		// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
		// requests different queue types

		vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

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

		vector<const char*> layerNames;
		layerNames.reserve(requestedLayers.size());
		for (const auto& name : requestedLayers)
		{
			layerNames.push_back(name.c_str());
		}

		vector<const char*> extensionNames;
		extensionNames.reserve(requestedExtensions.size());
		for (const auto& name : requestedExtensions)
		{
			extensionNames.push_back(name.c_str());
		}

		VkDeviceCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.pNext = &enabledDeviceFeatures2;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.pEnabledFeatures = nullptr;
		createInfo.enabledLayerCount = (uint32_t)layerNames.size();
		createInfo.ppEnabledLayerNames = layerNames.data();
		createInfo.enabledExtensionCount = (uint32_t)extensionNames.size();
		createInfo.ppEnabledExtensionNames = extensionNames.data();

		VkDevice vkDevice{ VK_NULL_HANDLE };
		ThrowIfFailed(vkCreateDevice(*physicalDevice, &createInfo, nullptr, &vkDevice));
		device = DeviceRef::Create(physicalDevice, vkDevice);

		// TODO - Delete me
		g_device = device;

		// Create semaphores
		CreateSemaphores();
	}


	void CreateVmaAllocator()
	{
		VmaAllocatorCreateInfo createInfo = {};
		createInfo.physicalDevice = *physicalDevice;
		createInfo.device = *device;
		
		VmaAllocator vmaAllocator{ VK_NULL_HANDLE };
		ThrowIfFailed(vmaCreateAllocator(&createInfo, &vmaAllocator));

		allocator = AllocatorRef::Create(instance, physicalDevice, device, vmaAllocator);
	}


	void InitSurface(HINSTANCE hInstance, HWND hWnd)
	{
		// Create surface
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.pNext = nullptr;
		surfaceCreateInfo.hinstance = hInstance;
		surfaceCreateInfo.hwnd = hWnd;

		VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };
		VkResult res = vkCreateWin32SurfaceKHR(*instance, &surfaceCreateInfo, nullptr, &vkSurface);

		if (res != VK_SUCCESS)
		{
			Utility::ExitFatal("Could not create surface!", "Fatal error");
		}
		surface = SurfaceRef::Create(instance, vkSurface);

		// Find a graphics queue that supports present
		const auto& queueFamilyIndices = GetGraphicsPresentQueueFamilyIndices(*surface);
		assert(!queueFamilyIndices.empty());

		// Get list of supported surface formats
		uint32_t formatCount;
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(GetPhysicalDevice(), GetSurface(), &formatCount, nullptr));
		assert(formatCount > 0);

		vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(GetPhysicalDevice(), GetSurface(), &formatCount, surfaceFormats.data()));

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
		auto oldSwapchain = swapchain;

		// Get physical device surface properties and formats
		VkSurfaceCapabilitiesKHR surfCaps;
		ThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(GetPhysicalDevice(), GetSurface(), &surfCaps));

		// Get available present modes
		uint32_t presentModeCount;
		ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(GetPhysicalDevice(), GetSurface(), &presentModeCount, nullptr));
		assert(presentModeCount > 0);

		vector<VkPresentModeKHR> presentModes(presentModeCount);
		ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(GetPhysicalDevice(), GetSurface(), &presentModeCount, presentModes.data()));

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
		uint32_t desiredNumberOfSwapchainImages = max(surfCaps.minImageCount + 1, NumSwapChainBuffers);
		if ((surfCaps.maxImageCount > 0) && (desiredNumberOfSwapchainImages > surfCaps.maxImageCount))
		{
			desiredNumberOfSwapchainImages = surfCaps.maxImageCount;
		}

		// Find the transformation of the surface
		VkSurfaceTransformFlagBitsKHR preTransform;
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

		VkImageUsageFlags usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

		// Set additional usage flag for blitting from the swapchain images if supported
		VkFormatProperties formatProps;
		vkGetPhysicalDeviceFormatProperties(GetPhysicalDevice(), vkFormat, &formatProps);
		if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)
		{
			usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		}

		// Create the swapchain
		VkSwapchainKHR oldsc = VK_NULL_HANDLE;
		if (oldSwapchain)
		{
			oldsc = *oldSwapchain;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.pNext = nullptr;
		createInfo.surface = *surface;
		createInfo.minImageCount = desiredNumberOfSwapchainImages;
		createInfo.imageFormat = vkFormat;
		createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
		createInfo.imageExtent = swapchainExtent;
		createInfo.imageUsage = usage;
		createInfo.preTransform = preTransform;
		createInfo.imageArrayLayers = 1u;
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.presentMode = swapchainPresentMode;
		createInfo.oldSwapchain = oldsc;
		// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
		createInfo.clipped = VK_TRUE;
		createInfo.compositeAlpha = compositeAlpha;

		VkSwapchainKHR vkSwapchain{ VK_NULL_HANDLE };
		ThrowIfFailed(vkCreateSwapchainKHR(*device, &createInfo, nullptr, &vkSwapchain));
		swapchain = SwapchainRef::Create(device, vkSwapchain);

		// Count actual swapchain images
		uint32_t imageCount{ 0 };
		ThrowIfFailed(vkGetSwapchainImagesKHR(*device, *swapchain, &imageCount, nullptr));

		// Get the swap chain images
		vector<VkImage> images(imageCount);
		ThrowIfFailed(vkGetSwapchainImagesKHR(*device, *swapchain, &imageCount, images.data()));

		swapchainImages.reserve(imageCount);
		for (auto image : images)
		{
			swapchainImages.push_back(ImageRef::Create(device, image));
		}
	}


	uint32_t AcquireNextImage()
	{
		uint32_t nextImageIndex = 0u;
		vkAcquireNextImageKHR(GetLogicalDevice(), GetSwapchain(), UINT64_MAX, *imageAcquireSemaphore.get(), VK_NULL_HANDLE, &nextImageIndex);
		return nextImageIndex;
	}


	void WaitForImageAcquisition(VkQueue queue)
	{
		VkPipelineStageFlags waitFlag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

		VkSemaphore waitSemaphore = *imageAcquireSemaphore.get();

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &waitSemaphore;
		submitInfo.pWaitDstStageMask = &waitFlag;
		submitInfo.signalSemaphoreCount = 0;
		submitInfo.pSignalSemaphores = nullptr;
		submitInfo.commandBufferCount = 0;
		submitInfo.pCommandBuffers = nullptr;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	}


	void UnblockPresent(VkQueue queue, VkSemaphore timelineSemaphore, uint64_t fenceWaitValue)
	{
		uint64_t dummy = 0;

		VkTimelineSemaphoreSubmitInfo timelineSubmitInfo;
		timelineSubmitInfo.sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO;
		timelineSubmitInfo.pNext = nullptr;
		timelineSubmitInfo.waitSemaphoreValueCount = 1;
		timelineSubmitInfo.pWaitSemaphoreValues = &fenceWaitValue;
		timelineSubmitInfo.signalSemaphoreValueCount = 1;
		timelineSubmitInfo.pSignalSemaphoreValues = &dummy;

		VkPipelineStageFlags waitFlag = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;

		VkSemaphore signalSemaphore = *presentSemaphore.get();

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = &timelineSubmitInfo;
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = &timelineSemaphore;
		submitInfo.pWaitDstStageMask = &waitFlag;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;
		submitInfo.commandBufferCount = 0;
		submitInfo.pCommandBuffers = nullptr;

		vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
	}


	bool IsExtensionSupported(const string& name) const
	{
		const char* c_str = name.c_str();
		return find_if(
			deviceExtensions.begin(),
			deviceExtensions.end(),
			[c_str](auto const& e) { return strcmp(e.extensionName, c_str) == 0; }) != deviceExtensions.end();
	}


	void GatherApplicationExtensions(bool optionalFeatures)
	{
		auto& requestedExtensions = optionalFeatures ? optionalExtensions : requiredExtensions;
		auto& requestedFeatures = optionalFeatures ? g_optionalFeatures : g_requiredFeatures;

#if 0
		// VK_KHR_shader_float16_int8
		if (requestedFeatures.shaderFloat16 || requestedFeatures.shaderInt8)
			requestedExtensions.insert(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
#endif

		// Add some required and optional extensions used by every Application
		if (optionalFeatures)
		{
			requestedExtensions.insert(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
		}
		else
		{
			requestedExtensions.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
		}
	}

	void ValidateApplicationExtensions()
	{
		vector<string> missingExtensions;

		// Check required extensions
		for (const auto& extName : requiredExtensions)
		{
			if (IsExtensionSupported(extName))
			{
				requestedExtensions.insert(extName);
			}
			else
			{
				missingExtensions.push_back(extName);
			}
		}

		// Report missing extensions and exit
		if (!missingExtensions.empty())
		{
			string errMsg;
			string errDetails;
			if (missingExtensions.size() > 1)
			{
				errMsg = "Required Vulkan Extensions Not Supported";
				errDetails = "This Application requires:\n ";
				for (size_t i = 0; i < missingExtensions.size(); ++i)
					errDetails += missingExtensions[i] + "\n";
				errDetails += "\n, which are unavailable.  You may need to update your GPU or graphics driver";
			}
			else
			{
				errMsg = "Required Vulkan Extension Not Supported";
				errDetails = "This Application requires:\n " + missingExtensions[0] + "\n, which is unavailable.  You may need to update your GPU or graphics driver";

			}
			ExitFatal(errMsg, errDetails);
		}

		// Check optional extensions
		for (const auto& extName : optionalExtensions)
		{
			if (IsExtensionSupported(extName))
			{
				requestedExtensions.insert(extName);
			}
		}

		// Now, hook up pointers for all the per-extension device features
		void** pNextSupported = &supportedDeviceFeatures1_2.pNext;
		void** pNextEnabled = &enabledDeviceFeatures1_2.pNext;

		for (const auto& extName : requestedExtensions)
		{
#if 0
			if (extName == VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)
			{
				*pNextSupported = &supportedExtendedFeatures.khrShaderFloat16Int8Features;
				pNextSupported = &supportedExtendedFeatures.khrShaderFloat16Int8Features.pNext;

				*pNextEnabled = &enabledExtendedFeatures.khrShaderFloat16Int8Features;
				pNextEnabled = &enabledExtendedFeatures.khrShaderFloat16Int8Features.pNext;
			}
#endif
		}
	}

	void EnableFeatures(bool optionalFeatures)
	{
		auto& requestedFeatures = optionalFeatures ? g_optionalFeatures : g_requiredFeatures;
		auto& enabledFeatures = const_cast<GraphicsFeatureSet&>(g_enabledFeatures);

		// Require timeline semaphores always
		if (!optionalFeatures)
		{
			TryEnableFeature(
				false,
				"Timeline Semaphore",
				supportedDeviceFeatures1_2.timelineSemaphore,
				enabledDeviceFeatures1_2.timelineSemaphore);
		}

		auto numFeatures = requestedFeatures.GetNumFeatures();
		for (auto i = 0; i < numFeatures; ++i)
		{
			const auto& requestedFeature = requestedFeatures[i];
			auto& enabledFeature = enabledFeatures[i];

			if (!requestedFeature)
				continue;

			const string& name = requestedFeature.GetName();

			switch (requestedFeature.GetFeature())
			{
			case GraphicsFeature::RobustBufferAccess:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.robustBufferAccess,
					enabledDeviceFeatures2.features.robustBufferAccess);
				break;

			case GraphicsFeature::FullDrawIndexUint32:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.fullDrawIndexUint32,
					enabledDeviceFeatures2.features.fullDrawIndexUint32);
				break;

			case GraphicsFeature::TextureCubeArray:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.imageCubeArray,
					enabledDeviceFeatures2.features.imageCubeArray);
				break;

			case GraphicsFeature::IndependentBlend:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.independentBlend,
					enabledDeviceFeatures2.features.independentBlend);
				break;

			case GraphicsFeature::GeometryShader:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.geometryShader,
					enabledDeviceFeatures2.features.geometryShader);
				break;

			case GraphicsFeature::TessellationShader:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.tessellationShader,
					enabledDeviceFeatures2.features.tessellationShader);
				break;

			case GraphicsFeature::SampleRateShading:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.sampleRateShading,
					enabledDeviceFeatures2.features.sampleRateShading);
				break;

			case GraphicsFeature::DualSrcBlend:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.dualSrcBlend,
					enabledDeviceFeatures2.features.dualSrcBlend);
				break;

			case GraphicsFeature::LogicOp:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.logicOp,
					enabledDeviceFeatures2.features.logicOp);
				break;

			case GraphicsFeature::DrawIndirectFirstInstance:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.drawIndirectFirstInstance,
					enabledDeviceFeatures2.features.drawIndirectFirstInstance);
				break;

			case GraphicsFeature::DepthClamp:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.depthClamp,
					enabledDeviceFeatures2.features.depthClamp);
				break;

			case GraphicsFeature::DepthBiasClamp:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.depthBiasClamp,
					enabledDeviceFeatures2.features.depthBiasClamp);
				break;

			case GraphicsFeature::FillModeNonSolid:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.fillModeNonSolid,
					enabledDeviceFeatures2.features.fillModeNonSolid);
				break;

			case GraphicsFeature::DepthBounds:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.depthBounds,
					enabledDeviceFeatures2.features.depthBounds);
				break;

			case GraphicsFeature::WideLines:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.wideLines,
					enabledDeviceFeatures2.features.wideLines);
				break;

			case GraphicsFeature::LargePoints:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.largePoints,
					enabledDeviceFeatures2.features.largePoints);
				break;

			case GraphicsFeature::AlphaToOne:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.alphaToOne,
					enabledDeviceFeatures2.features.alphaToOne);
				break;

			case GraphicsFeature::MultiViewport:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.multiViewport,
					enabledDeviceFeatures2.features.multiViewport);
				break;

			case GraphicsFeature::SamplerAnisotropy:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.samplerAnisotropy,
					enabledDeviceFeatures2.features.samplerAnisotropy);
				break;

			case GraphicsFeature::TextureCompressionETC2:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.textureCompressionETC2,
					enabledDeviceFeatures2.features.textureCompressionETC2);
				break;

			case GraphicsFeature::TextureCompressionASTC_LDR:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.textureCompressionASTC_LDR,
					enabledDeviceFeatures2.features.textureCompressionASTC_LDR);
				break;

			case GraphicsFeature::TextureCompressionBC:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.textureCompressionBC,
					enabledDeviceFeatures2.features.textureCompressionBC);
				break;

			case GraphicsFeature::OcclusionQueryPrecise:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.occlusionQueryPrecise,
					enabledDeviceFeatures2.features.occlusionQueryPrecise);
				break;

			case GraphicsFeature::PipelineStatisticsQuery:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.pipelineStatisticsQuery,
					enabledDeviceFeatures2.features.pipelineStatisticsQuery);
				break;

			case GraphicsFeature::VertexPipelineStoresAndAtomics:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.vertexPipelineStoresAndAtomics,
					enabledDeviceFeatures2.features.vertexPipelineStoresAndAtomics);
				break;

			case GraphicsFeature::PixelShaderStoresAndAtomics:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.fragmentStoresAndAtomics,
					enabledDeviceFeatures2.features.fragmentStoresAndAtomics);
				break;

			case GraphicsFeature::ShaderTessellationAndGeometryPointSize:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.shaderTessellationAndGeometryPointSize,
					enabledDeviceFeatures2.features.shaderTessellationAndGeometryPointSize);
				break;

			case GraphicsFeature::ShaderTextureGatherExtended:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.shaderImageGatherExtended,
					enabledDeviceFeatures2.features.shaderImageGatherExtended);
				break;
			case GraphicsFeature::ShaderUAVExtendedFormats:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.shaderStorageImageExtendedFormats,
					enabledDeviceFeatures2.features.shaderStorageImageExtendedFormats);
				break;

			case GraphicsFeature::ShaderClipDistance:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.shaderClipDistance,
					enabledDeviceFeatures2.features.shaderClipDistance);
				break;
			case GraphicsFeature::ShaderCullDistance:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.shaderCullDistance,
					enabledDeviceFeatures2.features.shaderCullDistance);
				break;
			case GraphicsFeature::ShaderFloat64:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.shaderFloat64,
					enabledDeviceFeatures2.features.shaderFloat64);
				break;
			case GraphicsFeature::ShaderFloat16:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures1_2.shaderFloat16,
					enabledDeviceFeatures1_2.shaderFloat16);
				break;
			case GraphicsFeature::ShaderInt64:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.shaderInt64,
					enabledDeviceFeatures2.features.shaderInt64);
				break;
			case GraphicsFeature::ShaderInt16:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.shaderInt16,
					enabledDeviceFeatures2.features.shaderInt16);
				break;
			case GraphicsFeature::ShaderInt8:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures1_2.shaderInt8,
					enabledDeviceFeatures1_2.shaderInt8);
				break;

			case GraphicsFeature::VariableMultisampleRate:
				enabledFeature = TryEnableFeature(
					optionalFeatures,
					name,
					supportedDeviceFeatures2.features.variableMultisampleRate,
					enabledDeviceFeatures2.features.variableMultisampleRate);
				break;
			}
		}
	}

	bool TryEnableFeature(bool optional, const string& name, const VkBool32& supportedFeature, VkBool32& enabledFeature)
	{
		bool supported = VK_TRUE == supportedFeature;
		if (!optional && !supported)
		{
			unsupportedRequiredFeatures.push_back(name);
			ExitFatal(
				"Required Feature Not Supported",
				"This Application requires " + name + ", which is unavailable.  You may need to update your GPU or graphics driver");
		}
		enabledFeature = supportedFeature;
		return supported;
	}

	VkInstance GetInstance() { return *instance.get(); }
	VkPhysicalDevice GetPhysicalDevice() { return *physicalDevice.get(); }
	VkDevice GetLogicalDevice() { return *device.get(); }
	VkSurfaceKHR GetSurface() { return *surface.get(); }
	VkSwapchainKHR GetSwapchain() { return *swapchain.get();
}

	// Shared pointers to core Vulkan objects
	shared_ptr<InstanceRef> instance;
	shared_ptr<PhysicalDeviceRef> physicalDevice;
	shared_ptr<DeviceRef> device;
	shared_ptr<AllocatorRef> allocator;
	shared_ptr<SurfaceRef> surface;
	shared_ptr<SwapchainRef> swapchain;
	shared_ptr<DebugReportCallbackRef> debugReportCallback;
	shared_ptr<SemaphoreRef> imageAcquireSemaphore;
	shared_ptr<SemaphoreRef> presentSemaphore;

	// Physical devices, properties, and extensions
	vector<VkPhysicalDevice> physicalDevices;

	VkPhysicalDeviceProperties				physicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties		physicalDeviceMemoryProperties;

	std::vector<VkExtensionProperties>		deviceExtensions;	

	// Required, optional, and enabled extensions
	set<string> supportedExtensions;
	set<string> requiredExtensions;
	set<string> optionalExtensions;
	set<string> requestedExtensions;
	vector<string> requestedLayers;

	vector<string> unsupportedRequiredFeatures;

	// Base features
	VkPhysicalDeviceFeatures2			supportedDeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, nullptr };
	VkPhysicalDeviceFeatures2			enabledDeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, nullptr };
	VkPhysicalDeviceVulkan12Features	supportedDeviceFeatures1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr };
	VkPhysicalDeviceVulkan12Features	enabledDeviceFeatures1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr };

	// Extended features
#if 0
	struct
	{
		VkPhysicalDeviceShaderFloat16Int8FeaturesKHR khrShaderFloat16Int8Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR, nullptr };
	} supportedExtendedFeatures, enabledExtendedFeatures;
#endif

	// Queues and queue families
	vector<VkQueueFamilyProperties> queueFamilyProperties;
	struct
	{
		uint32_t graphics;
		uint32_t compute;
		uint32_t transfer;
	} queueFamilyIndices;

	uint32_t presentQueueNodeIndex{ 0 };

	VkColorSpaceKHR colorSpace;
	
	// Swapchain images
	vector<shared_ptr<ImageRef>> swapchainImages;
};

} // namespace Kodiak


GraphicsDevice::~GraphicsDevice()
{}


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
	return m_platformData->GetMemoryTypeIndex(typeBits, properties, memTypeFound);
}


void GraphicsDevice::PlatformCreate()
{
	m_platformData = new PlatformData;

	m_platformData->CreateInstance(m_appName);

	m_platformData->SelectPhysicalDevice();
	m_deviceName = m_platformData->physicalDeviceProperties.deviceName;

	m_platformData->CreateLogicalDevice();

	m_platformData->CreateVmaAllocator();

	m_platformData->InitSurface(m_hinst, m_hwnd);
	m_platformData->CreateSwapChain(&m_width, &m_height, false /* vsync */);

	// Get ColorBuffers for swap chain images
	for (uint32_t i = 0; i < NumSwapChainBuffers; ++i)
	{
		m_swapChainBuffers[i] = make_shared<ColorBuffer>();
		VkImage image = *m_platformData->swapchainImages[i];
		auto handle = ResourceHandle::CreateNoDelete(image, VK_NULL_HANDLE, false);
		m_swapChainBuffers[i]->CreateFromSwapChain("Primary SwapChain Buffer", handle, m_width, m_height, BackBufferColorFormat);
	}

	g_commandManager.Create();

	// Acquire the first image from the swapchain, and have the graphics queue wait on it.
	m_currentBuffer = m_platformData->AcquireNextImage();
	m_platformData->WaitForImageAcquisition(g_commandManager.GetCommandQueue());
}


void GraphicsDevice::PlatformPresent()
{
	// Unblock present
	VkQueue commandQueue = g_commandManager.GetCommandQueue();
	auto& graphicsQueue = g_commandManager.GetGraphicsQueue();
	VkSemaphore timelineSemaphore = graphicsQueue.GetTimelineSemaphore();
	uint64_t fenceWaitValue = graphicsQueue.GetNextFenceValue() - 1;

	m_platformData->UnblockPresent(commandQueue, timelineSemaphore, fenceWaitValue);

	// Present
	VkSemaphore waitSemaphore = *m_platformData->presentSemaphore.get();
	VkSwapchainKHR swapchain = m_platformData->GetSwapchain();

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &m_currentBuffer;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitSemaphore;

	vkQueuePresentKHR(g_commandManager.GetCommandQueue(), &presentInfo);

	// Acquire the next image from the swapchain, and have the graphics queue wait on it.
	m_currentBuffer = m_platformData->AcquireNextImage();
	m_platformData->WaitForImageAcquisition(g_commandManager.GetCommandQueue());
}


void GraphicsDevice::PlatformDestroyData()
{
	delete m_platformData;
	m_platformData = nullptr;
}


void GraphicsDevice::PlatformDestroy()
{
	g_descriptorSetAllocator.DestroyAll();
}


VkFormatProperties GraphicsDevice::GetFormatProperties(Format format)
{
	VkFormat vkFormat = static_cast<VkFormat>(format);
	VkFormatProperties properties{};

	vkGetPhysicalDeviceFormatProperties(m_platformData->GetPhysicalDevice(), vkFormat, &properties);

	return properties;
}


uint32_t GraphicsDevice::GetQueueFamilyIndex(CommandListType type) const
{
	switch (type)
	{
	case CommandListType::Direct:
		return m_platformData->queueFamilyIndices.graphics;
		break;

	case CommandListType::Compute:
		return m_platformData->queueFamilyIndices.compute;
		break;

	case CommandListType::Copy:
		return m_platformData->queueFamilyIndices.transfer;
		break;

	default:
		return m_platformData->queueFamilyIndices.graphics;
		break;
	}
}


void GraphicsDevice::WaitForGpuIdle()
{
	g_commandManager.IdleGPU();
}


const DeviceHandle Kodiak::GetDevice()
{
	return *g_device.get();
}


VkFormatProperties Kodiak::GetFormatProperties(Format format)
{
	return g_graphicsDevice->GetFormatProperties(format);
}


uint32_t Kodiak::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound)
{
	return g_graphicsDevice->GetMemoryTypeIndex(typeBits, properties, memTypeFound);
}


#if ENABLE_VULKAN_DEBUG_MARKUP

void Kodiak::SetDebugName(VkInstance obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkPhysicalDevice obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkDevice obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkQueue obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkSemaphore obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkCommandBuffer obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkFence obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkDeviceMemory obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkBuffer obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkImage obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkEvent obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkQueryPool obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkBufferView obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkImageView obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkShaderModule obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkPipelineCache obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkPipelineLayout obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkRenderPass obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkPipeline obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkDescriptorSetLayout obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkSampler obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkDescriptorPool obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkDescriptorSet obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkFramebuffer obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkCommandPool obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkSurfaceKHR obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkSwapchainKHR obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT, name.c_str());
}


void Kodiak::SetDebugName(VkDebugReportCallbackEXT obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT, name.c_str());
}


#else


void Kodiak::SetDebugName(VkInstance obj, const string& name) {}
void Kodiak::SetDebugName(VkPhysicalDevice obj, const string& name) {}
void Kodiak::SetDebugName(VkDevice obj, const string& name) {}
void Kodiak::SetDebugName(VkQueue obj, const string& name) {}
void Kodiak::SetDebugName(VkSemaphore obj, const string& name) {}
void Kodiak::SetDebugName(VkCommandBuffer obj, const string& name) {}
void Kodiak::SetDebugName(VkFence obj, const string& name) {}
void Kodiak::SetDebugName(VkDeviceMemory obj, const string& name) {}
void Kodiak::SetDebugName(VkBuffer obj, const string& name) {}
void Kodiak::SetDebugName(VkImage obj, const string& name) {}
void Kodiak::SetDebugName(VkEvent obj, const string& name) {}
void Kodiak::SetDebugName(VkQueryPool obj, const string& name) {}
void Kodiak::SetDebugName(VkBufferView obj, const string& name) {}
void Kodiak::SetDebugName(VkImageView obj, const string& name) {}
void Kodiak::SetDebugName(VkShaderModule obj, const string& name) {}
void Kodiak::SetDebugName(VkPipelineCache obj, const string& name) {}
void Kodiak::SetDebugName(VkPipelineLayout obj, const string& name) {}
void Kodiak::SetDebugName(VkRenderPass obj, const string& name) {}
void Kodiak::SetDebugName(VkPipeline obj, const string& name) {}
void Kodiak::SetDebugName(VkDescriptorSetLayout obj, const string& name) {}
void Kodiak::SetDebugName(VkSampler obj, const string& name) {}
void Kodiak::SetDebugName(VkDescriptorPool obj, const string& name) {}
void Kodiak::SetDebugName(VkDescriptorSet obj, const string& name) {}
void Kodiak::SetDebugName(VkFramebuffer obj, const string& name) {}
void Kodiak::SetDebugName(VkCommandPool obj, const string& name) {}
void Kodiak::SetDebugName(VkSurfaceKHR obj, const string& name) {}
void Kodiak::SetDebugName(VkSwapchainKHR obj, const string& name) {}
void Kodiak::SetDebugName(VkDebugReportCallbackEXT obj, const string& name) {}

#endif