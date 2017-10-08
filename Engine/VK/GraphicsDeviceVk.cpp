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

#include "GraphicsDeviceVk.h"

#include "Utility.h"

#include "SwapChainVk.h"

#include <iostream>
#include <sstream>


using namespace Kodiak;
using namespace std;


GraphicsDevice::GraphicsDevice() = default;


GraphicsDevice::~GraphicsDevice() = default;


void GraphicsDevice::Initialize(const string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height)
{
	assert(!m_initialized);

	m_appName = appName;

	m_hinst = hInstance;
	m_hwnd = hWnd;

	m_width = m_destWidth = width;
	m_height = m_destHeight = height;

	CreateVulkanInstance();

#if ENABLE_VULKAN_VALIDATION
	// Enable validation for debugging

	// The report flags determine what type of messages for the layers will be displayed
	// For validating (debugging) an appplication the error and warning bits should suffice
	VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	// Additional flags include performance info, loader and layer debug messages, etc.
	InitializeDebugging(debugReportFlags, VK_NULL_HANDLE);
#endif

	SelectPhysicalDevice();

	// TODO: Setup features here

	CreateLogicalDevice();

	m_swapChain = make_unique<SwapChain>();
	m_swapChain->Connect(m_instance, m_physicalDevice, m_device);

	CreateSynchronizationPrimitives();

	m_swapChain->InitSurface(m_hinst, m_hwnd);

	m_swapChain->Create(&m_width, &m_height);

	m_initialized = true;
}


void GraphicsDevice::Destroy()
{
	assert(m_initialized);

	m_swapChain->Destroy();
	m_swapChain.reset();

	vkDestroySemaphore(m_device, m_semaphores.renderComplete, nullptr);
	vkDestroySemaphore(m_device, m_semaphores.textOverlayComplete, nullptr);
	vkDestroySemaphore(m_device, m_semaphores.presentComplete, nullptr);

	vkDestroyDevice(m_device, nullptr);
	m_device = VK_NULL_HANDLE;

#if ENABLE_VULKAN_VALIDATION
	FreeDebugCallback();
#endif

	vkDestroyInstance(m_instance, nullptr);
	m_instance = VK_NULL_HANDLE;

	m_initialized = false;
}


void GraphicsDevice::PrepareFrame()
{
	// Acquire the next image from the swap chain
	VkResult err = m_swapChain->AcquireNextImage(m_semaphores.presentComplete, &m_currentBuffer);

	ThrowIfFailed(err);

	// TODO:
	// Recreate the swapchain if it's no longer compatible with the surface (OUT_OF_DATE) or no longer optimal for presentation (SUBOPTIMAL)
	/*if ((err == VK_ERROR_OUT_OF_DATE_KHR) || (err == VK_SUBOPTIMAL_KHR))
	{
	WindowResize();
	}
	else
	{
	ThrowIfFailed(err);
	}*/
}


void GraphicsDevice::SubmitFrame()
{
	ThrowIfFailed(m_swapChain->QueuePresent(m_graphicsQueue, m_currentBuffer, m_semaphores.renderComplete));

	ThrowIfFailed(vkQueueWaitIdle(m_graphicsQueue));
}


void GraphicsDevice::CreateVulkanInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = m_appName.c_str();
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

	auto res = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
	if (VK_SUCCESS != res)
	{
		Utility::ExitFatal("Could not create Vulkan instance", "Fatal error");
	}
}


void GraphicsDevice::SelectPhysicalDevice()
{
	uint32_t gpuCount = 0;
	// Get number of available physical devices
	ThrowIfFailed(vkEnumeratePhysicalDevices(m_instance, &gpuCount, nullptr));
	assert(gpuCount > 0);
	// Enumerate devices
	vector<VkPhysicalDevice> physicalDevices(gpuCount);
	auto res = vkEnumeratePhysicalDevices(m_instance, &gpuCount, physicalDevices.data());
	if (res)
	{
		Utility::ExitFatal("Could not enumerate physical devices", "Fatal error");
	}

	// GPU selection

	m_physicalDevice = physicalDevices[m_gpuIndex];

	// Store properties (including limits), features and memory properties of the phyiscal device (so that examples can check against them)
	vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalDeviceProperties);
	vkGetPhysicalDeviceFeatures(m_physicalDevice, &m_physicalDeviceSupportedFeatures);
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_physicalDeviceMemoryProperties);
}


void GraphicsDevice::CreateLogicalDevice()
{
	// Desired queues need to be requested upon logical device creation
	// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
	// requests different queue types

	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

	// Queue family properties, used for setting up requested queues upon device creation
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, nullptr);
	assert(queueFamilyCount > 0);
	m_queueFamilyProperties.resize(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueFamilyCount, m_queueFamilyProperties.data());

	// Get list of supported extensions
	uint32_t extCount = 0;
	vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, nullptr);
	if (extCount > 0)
	{
		vector<VkExtensionProperties> extensions(extCount);
		if (vkEnumerateDeviceExtensionProperties(m_physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
		{
			for (auto ext : extensions)
			{
				m_supportedExtensions.push_back(ext.extensionName);
			}
		}
	}

	// Get queue family indices for the requested queue family types
	// Note that the indices may overlap depending on the implementation

	const float defaultQueuePriority = 0.0f;

	// Graphics queue
	m_queueFamilyIndices.graphics = GetQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
	VkDeviceQueueCreateInfo queueInfo{};
	queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueInfo.queueFamilyIndex = m_queueFamilyIndices.graphics;
	queueInfo.queueCount = 1;
	queueInfo.pQueuePriorities = &defaultQueuePriority;
	queueCreateInfos.push_back(queueInfo);

	// Dedicated compute queue
	m_queueFamilyIndices.compute = GetQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
	if (m_queueFamilyIndices.compute != m_queueFamilyIndices.graphics)
	{
		// If compute family index differs, we need an additional queue create info for the compute queue
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = m_queueFamilyIndices.compute;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueInfo);
	}

	// Dedicated transfer queue
	m_queueFamilyIndices.transfer = GetQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
	if ((m_queueFamilyIndices.transfer != m_queueFamilyIndices.graphics) && (m_queueFamilyIndices.transfer != m_queueFamilyIndices.compute))
	{
		// If compute family index differs, we need an additional queue create info for the compute queue
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = m_queueFamilyIndices.transfer;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueInfo);
	}

	// Create the logical device representation
	vector<const char*> deviceExtensions(m_enabledExtensions);
	deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);

	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());;
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
	deviceCreateInfo.pEnabledFeatures = &m_physicalDeviceEnabledFeatures;

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

	VkResult result = vkCreateDevice(m_physicalDevice, &deviceCreateInfo, nullptr, &m_device);
	ThrowIfFailed(result);

	// Get a graphics queue from the device
	vkGetDeviceQueue(m_device, m_queueFamilyIndices.graphics, 0, &m_graphicsQueue);

	// Find a suitable depth format
	//VkBool32 validDepthFormat = GetSupportedDepthFormat(s_physicalDevice, &s_depthFormat);
	//assert(validDepthFormat);
}


void GraphicsDevice::CreateSynchronizationPrimitives()
{
	// Create synchronization objects
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	semaphoreCreateInfo.pNext = nullptr;
	// Create a semaphore used to synchronize image presentation
	// Ensures that the image is displayed before we start submitting new commands to the queu
	ThrowIfFailed(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.presentComplete));
	// Create a semaphore used to synchronize command submission
	// Ensures that the image is not presented until all commands have been submitted and executed
	ThrowIfFailed(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.renderComplete));
	// Create a semaphore used to synchronize command submission
	// Ensures that the image is not presented until all commands for the text overlay have been sumbitted and executed
	// Will be inserted after the render complete semaphore if the text overlay is enabled
	ThrowIfFailed(vkCreateSemaphore(m_device, &semaphoreCreateInfo, nullptr, &m_semaphores.textOverlayComplete));

	// Set up submit info structure
	// Semaphores will stay the same during application lifetime
	// Command buffer submission info is set by each example
	m_submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	m_submitInfo.pNext = nullptr;
	m_submitInfo.pWaitDstStageMask = &m_submitPipelineStages;
	m_submitInfo.waitSemaphoreCount = 1;
	m_submitInfo.pWaitSemaphores = &m_semaphores.presentComplete;
	m_submitInfo.signalSemaphoreCount = 1;
	m_submitInfo.pSignalSemaphores = &m_semaphores.renderComplete;
}


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

	// The return value of this callback controls wether the Vulkan call that caused
	// the validation message will be aborted or not
	// We return VK_FALSE as we DON'T want Vulkan calls that cause a validation message 
	// (and return a VkResult) to abort
	// If you instead want to have calls abort, pass in VK_TRUE and the function will 
	// return VK_ERROR_VALIDATION_FAILED_EXT 
	return VK_FALSE;
}


void GraphicsDevice::InitializeDebugging(VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack)
{
	CreateDebugReportCallback = reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_instance, "vkCreateDebugReportCallbackEXT"));
	DestroyDebugReportCallback = reinterpret_cast<PFN_vkDestroyDebugReportCallbackEXT>(vkGetInstanceProcAddr(m_instance, "vkDestroyDebugReportCallbackEXT"));
	dbgBreakCallback = reinterpret_cast<PFN_vkDebugReportMessageEXT>(vkGetInstanceProcAddr(m_instance, "vkDebugReportMessageEXT"));

	VkDebugReportCallbackCreateInfoEXT dbgCreateInfo = {};
	dbgCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	dbgCreateInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
	dbgCreateInfo.flags = flags;

	VkResult err = CreateDebugReportCallback(
		m_instance,
		&dbgCreateInfo,
		nullptr,
		(callBack != VK_NULL_HANDLE) ? &callBack : &msgCallback);
	assert(!err);
}


void GraphicsDevice::FreeDebugCallback()
{
	if (msgCallback != VK_NULL_HANDLE)
	{
		DestroyDebugReportCallback(m_instance, msgCallback, nullptr);
	}
	msgCallback = VK_NULL_HANDLE;
}


uint32_t GraphicsDevice::GetQueueFamilyIndex(VkQueueFlagBits queueFlags)
{
	uint32_t index = 0;

	// Dedicated queue for compute
	// Try to find a queue family index that supports compute but not graphics
	if (queueFlags & VK_QUEUE_COMPUTE_BIT)
	{
		for (const auto& properties : m_queueFamilyProperties)
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
		for (const auto& properties : m_queueFamilyProperties)
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
	for (const auto& properties : m_queueFamilyProperties)
	{
		if (properties.queueFlags & queueFlags)
		{
			return index;
		}
		++index;
	}

	throw runtime_error("Could not find a matching queue family index");
}


bool GraphicsDevice::IsExtensionSupported(const string& name)
{
	return find(begin(m_supportedExtensions), end(m_supportedExtensions), name) != end(m_supportedExtensions);
}