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

#include "DebugVk.h"

#include "GraphicsDevice.h"
#include "InstanceVk.h"

#include <iostream>


using namespace Kodiak;
using namespace std;


#if ENABLE_VULKAN_VALIDATION
extern PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallback;
extern PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback;
extern PFN_vkDebugReportMessageEXT vkDebugReportMessage;

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


DebugReportCallback::DebugReportCallback(const shared_ptr<Instance>& instance, VkDebugReportFlagsEXT flags)
	: Reference(instance)
{
	Initialize(flags);
}


DebugReportCallback::~DebugReportCallback()
{
#if ENABLE_VULKAN_VALIDATION
	vkDestroyDebugReportCallback(*Get(), m_debugReportCallback, nullptr);
	m_debugReportCallback = VK_NULL_HANDLE;
#endif
}


void DebugReportCallback::Initialize(VkDebugReportFlagsEXT flags)
{
#if ENABLE_VULKAN_VALIDATION
	assert(vkCreateDebugReportCallback);
	assert(vkDestroyDebugReportCallback);

	VkDebugReportCallbackCreateInfoEXT createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CREATE_INFO_EXT;
	createInfo.pfnCallback = (PFN_vkDebugReportCallbackEXT)messageCallback;
	createInfo.flags = flags;

	ThrowIfFailed(vkCreateDebugReportCallback(*Get(), &createInfo, nullptr,	&m_debugReportCallback));
#endif
}


#if ENABLE_VULKAN_DEBUG_MARKUP
extern PFN_vkDebugMarkerSetObjectTagEXT vkDebugMarkerSetObjectTag;
extern PFN_vkDebugMarkerSetObjectNameEXT vkDebugMarkerSetObjectName;

extern bool g_debugMarkupAvailable;

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