//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

#define FORCE_VULKAN_VALIDATION 0
#define ENABLE_VULKAN_VALIDATION (_DEBUG || FORCE_VULKAN_VALIDATION)
//#define ENABLE_VULKAN_VALIDATION 0

#define FORCE_VULKAN_DEBUG_MARKUP 0
#define ENABLE_VULKAN_DEBUG_MARKUP (_DEBUG || _PROFILE || FORCE_VULKAN_DEBUG_MARKUP)

#define USE_VALIDATION_LAYER (ENABLE_VULKAN_VALIDATION || ENABLE_VULKAN_DEBUG_MARKUP)

#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>

#pragma comment(lib, "vulkan-1.lib")

#include "Extern\VulkanMemoryAllocator\vk_mem_alloc.h"

#include "HandleVk.h"
#include "RefCountingVk.h"

// Custom define for better code readability
#define VK_FLAGS_NONE 0
// Default fence timeout in nanoseconds
#define DEFAULT_FENCE_TIMEOUT 100000000000

const std::string s_apiName = "Vulkan";
const std::string s_apiPrefixString = "[Vulkan]";
const std::string s_defaultShaderPath = "Shaders\\SPIR-V";

inline void ThrowIfFailed(VkResult res)
{
	if (res != VK_SUCCESS)
	{
		throw;
	}
}

namespace Kodiak
{

using PlatformHandle = std::shared_ptr<VkBaseHandle>;
using ResourceHandle = VkResourceHandle::VkPointer;
using PsoHandle = VkPipeline;
using QueryHeapHandle = VkHandle<VkQueryPool>::VkPointer;

using SrvHandle = VkDescriptorHandle::VkPointer;
using UavHandle = VkDescriptorHandle::VkPointer;
struct IbvHandle {};
struct VbvHandle {};
using CbvHandle = VkDescriptorHandle::VkPointer;

using DsvHandle = VkHandle<VkImageView>::VkPointer;
using RtvHandle = VkHandle<VkImageView>::VkPointer;
using FboHandle = VkFramebufferHandle::VkPointer;

static const uint32_t NumSwapChainBuffers = 3;

// Debug name functions
void SetDebugName(VkInstance obj, const std::string& name);
void SetDebugName(VkPhysicalDevice obj, const std::string& name);
void SetDebugName(VkDevice obj, const std::string& name);
void SetDebugName(VkQueue obj, const std::string& name);
void SetDebugName(VkSemaphore obj, const std::string& name);
void SetDebugName(VkCommandBuffer obj, const std::string& name);
void SetDebugName(VkFence obj, const std::string& name);
void SetDebugName(VkDeviceMemory obj, const std::string& name);
void SetDebugName(VkBuffer obj, const std::string& name);
void SetDebugName(VkImage obj, const std::string& name);
void SetDebugName(VkEvent obj, const std::string& name);
void SetDebugName(VkQueryPool obj, const std::string& name);
void SetDebugName(VkBufferView obj, const std::string& name);
void SetDebugName(VkImageView obj, const std::string& name);
void SetDebugName(VkShaderModule obj, const std::string& name);
void SetDebugName(VkPipelineCache obj, const std::string& name);
void SetDebugName(VkPipelineLayout obj, const std::string& name);
void SetDebugName(VkRenderPass obj, const std::string& name);
void SetDebugName(VkPipeline obj, const std::string& name);
void SetDebugName(VkDescriptorSetLayout obj, const std::string& name);
void SetDebugName(VkSampler obj, const std::string& name);
void SetDebugName(VkDescriptorPool obj, const std::string& name);
void SetDebugName(VkDescriptorSet obj, const std::string& name);
void SetDebugName(VkFramebuffer obj, const std::string& name);
void SetDebugName(VkCommandPool obj, const std::string& name);
void SetDebugName(VkSurfaceKHR obj, const std::string& name);
void SetDebugName(VkSwapchainKHR obj, const std::string& name);
void SetDebugName(VkDebugReportCallbackEXT obj, const std::string& name);

} // namespace Kodiak