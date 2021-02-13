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

#include "Graphics\GraphicsFeatures.h"
#include "Graphics\PipelineState.h"
#include "Graphics\Shader.h"
#include "Graphics\Texture.h"
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


namespace Kodiak
{
GraphicsDevice* g_graphicsDevice = nullptr;
} // namespace Kodiak

extern Kodiak::CommandListManager g_commandManager;


#if USE_VALIDATION_LAYER
PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabel{ nullptr };
PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabel{ nullptr };
PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabel{ nullptr };
PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessenger{ nullptr };
PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger{ nullptr };
PFN_vkQueueBeginDebugUtilsLabelEXT vkQueueBeginDebugUtilsLabel{ nullptr };
PFN_vkQueueEndDebugUtilsLabelEXT vkQueueEndDebugUtilsLabel{ nullptr };
PFN_vkQueueInsertDebugUtilsLabelEXT vkQueueInsertDebugUtilsLabel{ nullptr };
PFN_vkSetDebugUtilsObjectNameEXT vkSetDebugUtilsObjectName{ nullptr };
PFN_vkSetDebugUtilsObjectTagEXT vkSetDebugUtilsObjectTag{ nullptr };
PFN_vkSubmitDebugUtilsMessageEXT vkSubmitDebugUtilsMessage{ nullptr };

#if ENABLE_VULKAN_VALIDATION
VkBool32 messageCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData)
{
	// Select prefix depending on flags passed to the callback
	// Note that multiple flags may be set for a single validation message
	string prefix("");

	// Message severity
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		prefix += "ERROR";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
	{
		prefix += "WARNING";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
	{
		prefix += "INFO";
	}
	else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
	{
		prefix += "VERBOSE";
	}

	// Message type
	if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT)
	{
		prefix += " [PERFORMANCE]";
	}
	else if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT)
	{
		prefix += " [VALIDATION]";
	}
	else if (messageTypes & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT)
	{
		prefix += " [GENERAL]";
	}

	prefix += ":";

	// Display message to default output (console/logcat)
	stringstream debugMessage;
	debugMessage << prefix << " [" << pCallbackData->pMessageIdName << "] Code " << pCallbackData->messageIdNumber << " : " << pCallbackData->pMessage;
	debugMessage << "\n";

	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		cerr << debugMessage.str();
	}
	else
	{
		cout << debugMessage.str();
	}

	OutputDebugString(debugMessage.str().c_str());
	if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
	{
		if (strstr(pCallbackData->pMessage, "VUID-vkDestroyBuffer-buffer-00922") == nullptr &&
			strstr(pCallbackData->pMessage, "VUID-vkFreeMemory-memory-00677") == nullptr)
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
#endif // ENABLE_VULKAN_VALIDATION

#endif // USE_VALIDATION_LAYER


#if ENABLE_VULKAN_DEBUG_MARKUP
bool g_debugMarkupAvailable = false;

void SetDebugName(uint64_t obj, VkObjectType objType, const char* name)
{
	if (g_debugMarkupAvailable)
	{
		VkDebugUtilsObjectNameInfoEXT nameInfo = {};
		nameInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT;
		nameInfo.pNext = nullptr;
		nameInfo.objectType = objType;
		nameInfo.objectHandle = obj;
		nameInfo.pObjectName = name;
		vkSetDebugUtilsObjectName(GetDevice(), &nameInfo);
	}
}
#endif


namespace
{

// TODO - Delete me
Microsoft::WRL::ComPtr<UVkDevice> g_device;

Format BackBufferColorFormat = Format::R8G8B8A8_UNorm;
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


namespace
{

void EnumeratePhysicalDevices(VkInstance instance, vector<VkPhysicalDevice>& physicalDevices)
{
	uint32_t gpuCount = 0;
	// Get number of available physical devices
	ThrowIfFailed(vkEnumeratePhysicalDevices(instance, &gpuCount, nullptr));
	assert(gpuCount > 0);

	// Enumerate devices
	physicalDevices.resize(gpuCount);
	auto res = vkEnumeratePhysicalDevices(instance, &gpuCount, physicalDevices.data());
	if (res)
	{
		Utility::ExitFatal("Could not enumerate physical devices", "Fatal error");
	}
}

} // anonymous namespace

GraphicsDevice::GraphicsDevice()
{
	m_supportedDeviceFeatures2.pNext = &m_supportedDeviceFeatures1_2;
	m_enabledDeviceFeatures2.pNext = &m_enabledDeviceFeatures1_2;
}


GraphicsDevice::~GraphicsDevice() = default;


void GraphicsDevice::Initialize(const string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height, Format colorFormat, Format depthFormat)
{
	assert(!m_platformData);

	g_graphicsDevice = this;

	m_appName = appName;

	m_hinst = hInstance;
	m_hwnd = hWnd;

	m_width = width;
	m_height = height;
	m_colorFormat = colorFormat;
	m_depthFormat = depthFormat;

	InitializeInternal();
}


void GraphicsDevice::Destroy()
{
	WaitForGpuIdle();

	CommandContext::DestroyAllContexts();

	PSO::DestroyAll();
	Shader::DestroyAll();
	RootSignature::DestroyAll();

	g_descriptorSetAllocator.DestroyAll();

	Texture::DestroyAll();

	for (int i = 0; i < NumSwapChainBuffers; ++i)
	{
		m_swapChainBuffers[i] = nullptr;
	}

	WaitForGpuIdle();

	// Flush pending deferred resources here
	ReleaseDeferredResources();
	assert(m_deferredResources.empty());

	g_commandManager.Destroy();

	g_graphicsDevice = nullptr;
}


void GraphicsDevice::SubmitFrame()
{
	Present();

	ReleaseDeferredResources();

	++m_frameNumber;
}


void GraphicsDevice::WaitForGpuIdle()
{
	g_commandManager.IdleGPU();
}


VkResult GraphicsDevice::CreateSemaphore(VkSemaphoreType semaphoreType, uint64_t initialValue, UVkSemaphore** ppSemaphore) const
{
	VkSemaphoreTypeCreateInfo typeCreateInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO };
	typeCreateInfo.pNext = nullptr;
	typeCreateInfo.semaphoreType = semaphoreType;
	typeCreateInfo.initialValue = initialValue;

	VkSemaphoreCreateInfo createInfo = { VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
	createInfo.pNext = &typeCreateInfo;
	createInfo.flags = 0;

	VkSemaphore vkSemaphore{ VK_NULL_HANDLE };
	auto res = vkCreateSemaphore(m_device->Get(), &createInfo, nullptr, &vkSemaphore);
	
	*ppSemaphore = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppSemaphore = new UVkSemaphore(m_device.Get(), vkSemaphore);
		(*ppSemaphore)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateAllocator(UVmaAllocator** ppAllocator) const
{
	VmaAllocatorCreateInfo createInfo = {};
	createInfo.physicalDevice = m_physicalDevice->Get();
	createInfo.device = m_device->Get();

	VmaAllocator vmaAllocator{ VK_NULL_HANDLE };
	auto res = vmaCreateAllocator(&createInfo, &vmaAllocator);

	*ppAllocator = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppAllocator = new UVmaAllocator(m_device.Get(), vmaAllocator);
		(*ppAllocator)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateQueryPool(QueryHeapType type, uint32_t queryCount, UVkQueryPool** ppPool) const
{
	VkQueryPoolCreateInfo createInfo = { VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO  };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.queryCount = queryCount;
	createInfo.queryType = QueryHeapTypeToVulkan(type);
	createInfo.pipelineStatistics = VK_QUERY_PIPELINE_STATISTIC_FLAG_BITS_MAX_ENUM;

	VkQueryPool vkQueryPool = VK_NULL_HANDLE;
	auto res = vkCreateQueryPool(m_device->Get(), &createInfo, nullptr, &vkQueryPool);

	*ppPool = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppPool = new UVkQueryPool(m_device.Get(), vkQueryPool);
		(*ppPool)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateCommandPool(uint32_t queueFamilyIndex, UVkCommandPool** ppPool) const
{
	VkCommandPoolCreateInfo createInfo{ VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	createInfo.queueFamilyIndex = queueFamilyIndex;

	VkCommandPool vkCommandPool = VK_NULL_HANDLE;
	auto res = vkCreateCommandPool(m_device->Get(), &createInfo, nullptr, &vkCommandPool);

	*ppPool = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppPool = new UVkCommandPool(m_device.Get(), vkCommandPool);
		(*ppPool)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateImageView(UVkImage* uimage, ResourceType type, Format format, bool forceColorAspect, uint32_t baseMipLevel, uint32_t mipCount, uint32_t baseArraySlice, uint32_t arraySize, UVkImageView** ppImageView) const
{
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.viewType = GetImageViewType(type);
	createInfo.format = static_cast<VkFormat>(format);
	if (IsColorFormat(format))
	{
		createInfo.components.r = VK_COMPONENT_SWIZZLE_R;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_G;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_B;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_A;
	}
	createInfo.subresourceRange = {};
	createInfo.subresourceRange.aspectMask = forceColorAspect ? VK_IMAGE_ASPECT_COLOR_BIT : GetAspectFlagsFromFormat(format);
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = mipCount;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = type == ResourceType::Texture3D ? 1 : arraySize;
	createInfo.image = uimage->Get();

	VkImageView vkImageView = VK_NULL_HANDLE;
	auto res = vkCreateImageView(m_device->Get(), &createInfo, nullptr, &vkImageView);

	*ppImageView = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppImageView = new UVkImageView(m_device.Get(), uimage, vkImageView);
		(*ppImageView)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateBufferView(UVkBuffer* ubuffer, ResourceType type, Format format, uint32_t offsetInBytes, uint32_t sizeInBytes, UVkBufferView** ppBufferView) const
{
	VkBufferViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.buffer = ubuffer->Get();
	createInfo.offset = offsetInBytes;
	createInfo.range = sizeInBytes;
	createInfo.format = static_cast<VkFormat>(format);

	VkBufferView vkBufferView = VK_NULL_HANDLE;
	auto res = vkCreateBufferView(m_device->Get(), &createInfo, nullptr, &vkBufferView);

	*ppBufferView = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppBufferView = new UVkBufferView(m_device.Get(), ubuffer, vkBufferView);
		(*ppBufferView)->AddRef();
	}

	return res;
}



// TODO Move this elsewhere?
static bool QueryLinearTilingFeature(VkFormatProperties properties, VkFormatFeatureFlagBits flags)
{
	return (properties.linearTilingFeatures & flags) != 0;
}


static bool QueryOptimalTilingFeature(VkFormatProperties properties, VkFormatFeatureFlagBits flags)
{
	return (properties.optimalTilingFeatures & flags) != 0;
}


static bool QueryBufferFeature(VkFormatProperties properties, VkFormatFeatureFlagBits flags)
{
	return (properties.bufferFeatures & flags) != 0;
}



VkResult GraphicsDevice::CreateRenderPass(const vector<ColorBufferPtr>& colorBuffers, DepthBufferPtr depthBuffer, UVkRenderPass** ppRenderPass) const
{
	vector<VkAttachmentDescription> attachments(colorBuffers.size() + 1);
	vector<uint32_t> regToAttachmentIndex(attachments.size(), VK_ATTACHMENT_UNUSED);

	// Process color targets
	uint32_t rtCount = 0;
	uint32_t numSamples = 1;
	for (uint32_t i = 0; i < uint32_t(colorBuffers.size()); ++i)
	{
		assert(colorBuffers[i] != nullptr);

		auto format = colorBuffers[i]->GetFormat();
		numSamples = colorBuffers[i]->GetNumSamples();
		if (format != Format::Unknown)
		{
			auto& desc = attachments[rtCount];
			regToAttachmentIndex[i] = rtCount;
			++rtCount;

			desc.flags = 0;
			desc.format = static_cast<VkFormat>(format);
			desc.samples = SamplesToFlags(numSamples);
			desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // This is a color attachment
			desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // This is a color attachment
			desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}
	}

	bool hasColor = rtCount > 0;
	bool hasDepth = false;

	// Process depth target
	if (depthBuffer && depthBuffer->GetFormat() != Format::Unknown)
	{
		auto& depthDesc = attachments[rtCount];
		regToAttachmentIndex.back() = rtCount;
		rtCount++;

		depthDesc.flags = 0;
		depthDesc.format = static_cast<VkFormat>(depthBuffer->GetFormat());
		depthDesc.samples = SamplesToFlags(numSamples);
		depthDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		depthDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
		depthDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		depthDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		hasDepth = true;
	}

	// Init subpass
	VkSubpassDescription subpassDesc = {};
	subpassDesc.flags = 0;

	vector<VkAttachmentReference> attachmentRefs(attachments.size());

	if (hasDepth)
	{
		auto& depthRef = attachmentRefs.back();
		depthRef.attachment = regToAttachmentIndex.back();
		depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		subpassDesc.pDepthStencilAttachment = &attachmentRefs.back();
	}

	if (hasColor)
	{
		for (uint32_t i = 0; i < uint32_t(colorBuffers.size()); ++i)
		{
			auto& ref = attachmentRefs[i];
			ref.attachment = regToAttachmentIndex[i];
			ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		}

		subpassDesc.colorAttachmentCount = uint32_t(colorBuffers.size());
		subpassDesc.pColorAttachments = attachmentRefs.data();
	}

	// Build renderpass
	VkRenderPassCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.attachmentCount = rtCount;
	createInfo.pAttachments = attachments.data();
	createInfo.subpassCount = 1;
	createInfo.pSubpasses = &subpassDesc;
	createInfo.dependencyCount = 0;
	createInfo.pDependencies = nullptr;

	VkRenderPass vkRenderPass = VK_NULL_HANDLE;
	auto res = vkCreateRenderPass(GetDevice(), &createInfo, nullptr, &vkRenderPass);

	*ppRenderPass = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppRenderPass = new UVkRenderPass(m_device.Get(), vkRenderPass);
		(*ppRenderPass)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateFramebuffer(const vector<ColorBufferPtr>& colorBuffers, DepthBufferPtr depthBuffer, VkRenderPass renderPass, UVkFramebuffer** ppFramebuffer) const
{
	const uint32_t numColorBuffers = uint32_t(colorBuffers.size());
	uint32_t rtCount = numColorBuffers;
	rtCount += depthBuffer ? 1 : 0;

	assert(rtCount > 0);

	uint32_t width = 0;
	uint32_t height = 0;
	if (numColorBuffers > 0)
	{
		width = colorBuffers[0]->GetWidth();
		height = colorBuffers[0]->GetHeight();
	}
	else
	{
		width = depthBuffer->GetWidth();
		height = depthBuffer->GetHeight();
	}

	vector<VkImageView> attachments(rtCount);

	uint32_t attachmentCount = 0;
	for (uint32_t i = 0; i < numColorBuffers; ++i)
	{
		attachments[i] = colorBuffers[i]->GetRTV().GetImageView();
		++attachmentCount;
	}

	if (depthBuffer)
	{
		attachments[attachmentCount] = depthBuffer->GetDSV().GetImageView();
		++attachmentCount;
	}

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = nullptr;
	frameBufferCreateInfo.renderPass = renderPass;
	frameBufferCreateInfo.attachmentCount = attachmentCount;
	frameBufferCreateInfo.pAttachments = attachments.data();
	frameBufferCreateInfo.width = width;
	frameBufferCreateInfo.height = height;
	frameBufferCreateInfo.layers = 1;

	// TODO - Move this to GraphicsDevice
	VkFramebuffer vkFramebuffer = VK_NULL_HANDLE;
	auto res = vkCreateFramebuffer(GetDevice(), &frameBufferCreateInfo, nullptr, &vkFramebuffer);

	*ppFramebuffer = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppFramebuffer = new UVkFramebuffer(m_device.Get(), vkFramebuffer);
		(*ppFramebuffer)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateBuffer(const string& name, const BufferDesc& desc, UVkBuffer** ppBuffer) const
{
	VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	createInfo.size = desc.bufferSizeInBytes;
	createInfo.usage = GetBufferUsageFlags(desc.type) | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.flags = GetMemoryFlags(desc.access);
	allocCreateInfo.usage = GetMemoryUsage(desc.access);

	VkBuffer vkBuffer = VK_NULL_HANDLE;
	VmaAllocation vmaBufferAlloc = VK_NULL_HANDLE;

	auto res = vmaCreateBuffer(m_allocator->Get(), &createInfo, &allocCreateInfo, &vkBuffer, &vmaBufferAlloc, nullptr);

	*ppBuffer = nullptr;
	if (res == VK_SUCCESS)
	{
		SetDebugName(vkBuffer, name);

		*ppBuffer = new UVkBuffer(m_device.Get(), m_allocator.Get(), vkBuffer, vmaBufferAlloc);
		(*ppBuffer)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateImage(const string& name, const ImageDesc& desc, UVkImage** ppImage) const
{
	VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
	imageCreateInfo.pNext = nullptr;
	imageCreateInfo.flags = GetImageCreateFlags(desc.type);
	imageCreateInfo.imageType = GetImageType(desc.type);
	imageCreateInfo.format = static_cast<VkFormat>(desc.format);
	imageCreateInfo.extent = { desc.width, desc.height, desc.GetDepth() };
	imageCreateInfo.mipLevels = desc.numMips;
	imageCreateInfo.arrayLayers = desc.GetArraySize();
	imageCreateInfo.samples = SamplesToFlags(desc.numSamples);
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.usage = GetImageUsageFlags(desc.usage);

	// Remove storage flag if this format doesn't support it.
	// TODO - Make a table with all the format properties?
	VkFormatProperties properties = GetFormatProperties(desc.format);
	if (!QueryOptimalTilingFeature(properties, VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT))
	{
		imageCreateInfo.usage &= ~VK_IMAGE_USAGE_STORAGE_BIT;
	}

	VmaAllocationCreateInfo imageAllocCreateInfo = {};
	imageAllocCreateInfo.flags = GetMemoryFlags(desc.access);
	imageAllocCreateInfo.usage = GetMemoryUsage(desc.access);

	VkImage vkImage = VK_NULL_HANDLE;
	VmaAllocation vmaAllocation = VK_NULL_HANDLE;
	auto res = vmaCreateImage(m_allocator->Get(), &imageCreateInfo, &imageAllocCreateInfo, &vkImage, &vmaAllocation, nullptr);

	*ppImage = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppImage = new UVkImage(m_device.Get(), m_allocator.Get(), vkImage, vmaAllocation);
		(*ppImage)->AddRef();

		SetDebugName(vkImage, name);
	}

	return res;
}


VkResult GraphicsDevice::CreatePipelineCache(UVkPipelineCache** ppPipelineCache) const
{
	VkPipelineCacheCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.initialDataSize = 0;
	createInfo.pInitialData = nullptr;

	VkPipelineCache vkPipelineCache = VK_NULL_HANDLE;
	auto res = vkCreatePipelineCache(m_device->Get(), &createInfo, nullptr, &vkPipelineCache);

	*ppPipelineCache = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppPipelineCache = new UVkPipelineCache(m_device.Get(), vkPipelineCache);
		(*ppPipelineCache)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo, UVkPipeline** ppPipeline) const
{
	VkPipeline vkPipeline = VK_NULL_HANDLE;
	auto res = vkCreateGraphicsPipelines(m_device->Get(), m_pipelineCache->Get(), 1, &createInfo, nullptr, &vkPipeline);

	*ppPipeline = nullptr;
	if(res == VK_SUCCESS)
	{
		*ppPipeline = new UVkPipeline(m_device.Get(), m_pipelineCache.Get(), vkPipeline);
		(*ppPipeline)->AddRef();
	}

	return res;
}


VkResult GraphicsDevice::CreateComputePipeline(const VkComputePipelineCreateInfo& createInfo, UVkPipeline** ppPipeline) const
{
	VkPipeline vkPipeline = VK_NULL_HANDLE;
	auto res = vkCreateComputePipelines(m_device->Get(), m_pipelineCache->Get(), 1, &createInfo, nullptr, &vkPipeline);

	*ppPipeline = nullptr;
	if (res == VK_SUCCESS)
	{
		*ppPipeline = new UVkPipeline(m_device.Get(), m_pipelineCache.Get(), vkPipeline);
		(*ppPipeline)->AddRef();
	}

	return res;
}


ColorBufferPtr GraphicsDevice::GetBackBuffer(uint32_t index) const
{
	assert(index < NumSwapChainBuffers);
	return m_swapChainBuffers[index];
}


void GraphicsDevice::ReleaseResource(UVkImage* image)
{
	uint64_t nextFence = g_commandManager.GetGraphicsQueue().GetNextFenceValue();

	DeferredReleaseResource resource{ nextFence, image, nullptr };
	m_deferredResources.emplace_back(resource);
}


void GraphicsDevice::ReleaseResource(UVkBuffer* buffer)
{
	uint64_t nextFence = g_commandManager.GetGraphicsQueue().GetNextFenceValue();

	DeferredReleaseResource resource{ nextFence, nullptr, buffer };
	m_deferredResources.emplace_back(resource);
}


uint32_t GraphicsDevice::GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const
{
	for (uint32_t i = 0; i < m_physicalDeviceMemoryProperties.memoryTypeCount; i++)
	{
		if ((typeBits & 1) == 1)
		{
			if ((m_physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
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


VkFormatProperties GraphicsDevice::GetFormatProperties(Format format) const
{
	VkFormat vkFormat = static_cast<VkFormat>(format);
	VkFormatProperties properties{};

	vkGetPhysicalDeviceFormatProperties(m_physicalDevice->Get(), vkFormat, &properties);

	return properties;
}


uint32_t GraphicsDevice::GetQueueFamilyIndex(CommandListType type) const
{
	switch (type)
	{
	case CommandListType::Direct:
		return m_queueFamilyIndices.graphics;
		break;

	case CommandListType::Compute:
		return m_queueFamilyIndices.compute;
		break;

	case CommandListType::Copy:
		return m_queueFamilyIndices.transfer;
		break;

	default:
		return m_queueFamilyIndices.graphics;
		break;
	}
}


void GraphicsDevice::ReleaseDeferredResources()
{
	auto resourceIt = m_deferredResources.begin();
	while (resourceIt != m_deferredResources.end())
	{
		if (g_commandManager.IsFenceComplete(resourceIt->fenceValue))
		{
			resourceIt = m_deferredResources.erase(resourceIt);
		}
		else
		{
			++resourceIt;
		}
	}
}


void GraphicsDevice::InitializeInternal()
{
	CreateInstance();

	SelectPhysicalDevice();
	m_deviceName = m_physicalDeviceProperties.deviceName;

	CreateLogicalDevice();

	ThrowIfFailed(CreateAllocator(&m_allocator));

	InitSurface();
	CreateSwapChain();

	// Get ColorBuffers for swap chain images
	for (uint32_t i = 0; i < NumSwapChainBuffers; ++i)
	{
		m_swapChainBuffers[i] = make_shared<ColorBuffer>();
		VkImage image = m_swapchainImages[i]->Get();
		auto uimage = new UVkImage(m_device.Get(), image);
		m_swapChainBuffers[i]->CreateFromSwapChain("Primary SwapChain Buffer", uimage, m_width, m_height, BackBufferColorFormat);
	}

	g_commandManager.Create();

	// Acquire the first image from the swapchain, and have the graphics queue wait on it.
	m_currentBuffer = AcquireNextImage();
	WaitForImageAcquisition(g_commandManager.GetCommandQueue());
}


void GraphicsDevice::Present()
{
	// Unblock present
	VkQueue commandQueue = g_commandManager.GetCommandQueue();
	auto& graphicsQueue = g_commandManager.GetGraphicsQueue();
	VkSemaphore timelineSemaphore = graphicsQueue.GetTimelineSemaphore();
	uint64_t fenceWaitValue = graphicsQueue.GetNextFenceValue() - 1;

	UnblockPresent(commandQueue, timelineSemaphore, fenceWaitValue);

	// Present
	VkSemaphore waitSemaphore = m_presentSemaphore->Get();
	VkSwapchainKHR swapchain = m_swapchain->Get();

	VkPresentInfoKHR presentInfo = { VK_STRUCTURE_TYPE_PRESENT_INFO_KHR };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &m_currentBuffer;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &waitSemaphore;

	vkQueuePresentKHR(g_commandManager.GetCommandQueue(), &presentInfo);

	// Acquire the next image from the swapchain, and have the graphics queue wait on it.
	m_currentBuffer = AcquireNextImage();
	WaitForImageAcquisition(g_commandManager.GetCommandQueue());
}


void GraphicsDevice::CreateInstance()
{
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = m_appName.c_str();
	appInfo.pEngineName = "Kodiak";
	appInfo.apiVersion = VK_API_VERSION_1_2;

	const vector<const char*> instanceExtensions =
	{
#if USE_VALIDATION_LAYER
			VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
#endif
#if ENABLE_VULKAN_VALIDATION
			VK_EXT_VALIDATION_FEATURES_EXTENSION_NAME,
#endif
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME
	};

	const vector<const char*> instanceLayers =
	{
#if USE_VALIDATION_LAYER
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

	m_instance = new UVkInstance(vkInstance);
}


void GraphicsDevice::SelectPhysicalDevice()
{
	// GPU selection
	vector<VkPhysicalDevice> physicalDevices;
	EnumeratePhysicalDevices(m_instance->Get(), physicalDevices);
	m_physicalDevice = new UVkPhysicalDevice(m_instance.Get(), physicalDevices[0]);

	// Get available physical device properties and features
	GetPhysicalDeviceProperties();

	// Initialize validation callbacks
	InitializeValidation();

	// Record which extensions are required or optional, based on Application config
	GatherApplicationExtensions(false);
	GatherApplicationExtensions(true);
	ValidateApplicationExtensions();

	// Enabled required and optional features, as requested by the application
	EnableFeatures(false);
	EnableFeatures(true);

	// Report missing features and exit
	if (!m_unsupportedRequiredFeatures.empty())
	{
		string errMsg;
		string errDetails;
		if (m_unsupportedRequiredFeatures.size() > 1)
		{
			errMsg = "Required Features Not Supported";
			errDetails = "This Application requires:\n ";
			for (size_t i = 0; i < m_unsupportedRequiredFeatures.size(); ++i)
				errDetails += m_unsupportedRequiredFeatures[i] + "\n";
			errDetails += "\n, which are unavailable.  You may need to update your GPU or graphics driver";
		}
		else
		{
			errMsg = "Required Feature Not Supported";
			errDetails = "This Application requires:\n " + m_unsupportedRequiredFeatures[0] + "\n, which is unavailable.  You may need to update your GPU or graphics driver";

		}
		ExitFatal(errMsg, errDetails);
	}
}


void GraphicsDevice::CreateLogicalDevice()
{
	// Desired queues need to be requested upon logical device creation
	// Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
	// requests different queue types

	vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

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
		// If compute family index differs, we need an additional queue create info for the transfer queue
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = m_queueFamilyIndices.transfer;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueInfo);
	}

	vector<const char*> layerNames;
	layerNames.reserve(m_requestedLayers.size());
	for (const auto& name : m_requestedLayers)
	{
		layerNames.push_back(name.c_str());
	}

	vector<const char*> extensionNames;
	extensionNames.reserve(m_requestedExtensions.size());
	for (const auto& name : m_requestedExtensions)
	{
		extensionNames.push_back(name.c_str());
	}

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pNext = &m_enabledDeviceFeatures2;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = nullptr;
	createInfo.enabledLayerCount = (uint32_t)layerNames.size();
	createInfo.ppEnabledLayerNames = layerNames.data();
	createInfo.enabledExtensionCount = (uint32_t)extensionNames.size();
	createInfo.ppEnabledExtensionNames = extensionNames.data();

	VkDevice vkDevice{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateDevice(m_physicalDevice->Get(), &createInfo, nullptr, &vkDevice));
	m_device = new UVkDevice(m_physicalDevice.Get(), vkDevice);

	// TODO - Delete me
	g_device = m_device;

	// Create semaphores
	ThrowIfFailed(CreateSemaphore(VK_SEMAPHORE_TYPE_BINARY, 0, &m_imageAcquireSemaphore));
	ThrowIfFailed(CreateSemaphore(VK_SEMAPHORE_TYPE_BINARY, 0, &m_presentSemaphore));

	// Create pipeline cache
	ThrowIfFailed(CreatePipelineCache(&m_pipelineCache));
}


void GraphicsDevice::InitSurface()
{
	// Create surface
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.pNext = nullptr;
	surfaceCreateInfo.hinstance = m_hinst;
	surfaceCreateInfo.hwnd = m_hwnd;

	VkSurfaceKHR vkSurface{ VK_NULL_HANDLE };
	VkResult res = vkCreateWin32SurfaceKHR(m_instance->Get(), &surfaceCreateInfo, nullptr, &vkSurface);

	if (res != VK_SUCCESS)
	{
		Utility::ExitFatal("Could not create surface!", "Fatal error");
	}
	m_surface = new UVkSurface(m_instance.Get(), vkSurface);

	// Find a graphics queue that supports present
	const auto& queueFamilyIndices = GetGraphicsPresentQueueFamilyIndices(m_surface->Get());
	assert(!queueFamilyIndices.empty());

	// Get list of supported surface formats
	uint32_t formatCount;
	ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice->Get(), m_surface->Get(), &formatCount, nullptr));
	assert(formatCount > 0);

	vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice->Get(), m_surface->Get(), &formatCount, surfaceFormats.data()));

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferred format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		BackBufferColorFormat = MapVulkanFormatToEngine(VK_FORMAT_B8G8R8A8_UNORM);
		m_colorSpace = surfaceFormats[0].colorSpace;
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
				m_colorSpace = surfaceFormat.colorSpace;
				found_B8G8R8A8_UNORM = true;
				break;
			}
		}

		// in case VK_FORMAT_B8G8R8A8_UNORM is not available
		// select the first available color format
		if (!found_B8G8R8A8_UNORM)
		{
			BackBufferColorFormat = MapVulkanFormatToEngine(surfaceFormats[0].format);
			m_colorSpace = surfaceFormats[0].colorSpace;
		}
	}
	m_colorFormat = BackBufferColorFormat;
}


void GraphicsDevice::CreateSwapChain()
{
	auto oldSwapchain = m_swapchain;

	VkPhysicalDevice physicalDevice = m_physicalDevice->Get();
	VkSurfaceKHR surface = m_surface->Get();

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
		swapchainExtent.width = m_width;
		swapchainExtent.height = m_height;
	}
	else
	{
		// If the surface size is defined, the swap chain size must match
		swapchainExtent = surfCaps.currentExtent;
		m_width = surfCaps.currentExtent.width;
		m_height = surfCaps.currentExtent.height;
	}

	// Select a present mode for the swapchain

	// The VK_PRESENT_MODE_FIFO_KHR mode must always be present as per spec
	// This mode waits for the vertical blank ("v-sync")
	VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	// If v-sync is not requested, try to find a mailbox mode
	// It's the lowest latency non-tearing present mode available
	if (!m_vsync)
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
	vkGetPhysicalDeviceFormatProperties(physicalDevice, vkFormat, &formatProps);
	if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)
	{
		usage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	// Create the swapchain
	VkSwapchainKHR oldsc = VK_NULL_HANDLE;
	if (oldSwapchain)
	{
		oldsc = oldSwapchain->Get();
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.surface = surface;
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
	ThrowIfFailed(vkCreateSwapchainKHR(m_device->Get(), &createInfo, nullptr, &vkSwapchain));
	m_swapchain = new UVkSwapchain(m_device.Get(), vkSwapchain);

	// Count actual swapchain images
	uint32_t imageCount{ 0 };
	ThrowIfFailed(vkGetSwapchainImagesKHR(m_device->Get(), m_swapchain->Get(), &imageCount, nullptr));

	// Get the swap chain images
	vector<VkImage> images(imageCount);
	ThrowIfFailed(vkGetSwapchainImagesKHR(m_device->Get(), m_swapchain->Get(), &imageCount, images.data()));

	m_swapchainImages.reserve(imageCount);
	for (auto image : images)
	{
		m_swapchainImages.push_back(new UVkImage(m_device.Get(), image));
	}
}


uint32_t GraphicsDevice::AcquireNextImage()
{
	uint32_t nextImageIndex = 0u;
	vkAcquireNextImageKHR(m_device->Get(), m_swapchain->Get(), UINT64_MAX, m_imageAcquireSemaphore->Get(), VK_NULL_HANDLE, &nextImageIndex);
	return nextImageIndex;
}


void GraphicsDevice::WaitForImageAcquisition(VkQueue queue)
{
	VkPipelineStageFlags waitFlag = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;

	VkSemaphore waitSemaphore = m_imageAcquireSemaphore->Get();

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


void GraphicsDevice::UnblockPresent(VkQueue queue, VkSemaphore timelineSemaphore, uint64_t fenceWaitValue)
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

	VkSemaphore signalSemaphore = m_presentSemaphore->Get();

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


void GraphicsDevice::GetPhysicalDeviceProperties()
{
	VkPhysicalDevice physicalDevice = m_physicalDevice->Get();

	// Get device and memory properties
	vkGetPhysicalDeviceProperties(physicalDevice, &m_physicalDeviceProperties);
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &m_physicalDeviceMemoryProperties);

	// Get device features
	vkGetPhysicalDeviceFeatures2(physicalDevice, &m_supportedDeviceFeatures2);

	// Get list of supported extensions
	uint32_t extCount = 0;
	ThrowIfFailed(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr));
	if (extCount > 0)
	{
		m_deviceExtensions.resize(extCount);
		ThrowIfFailed(vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &m_deviceExtensions.front()));
	}

	// Get available queue family properties
	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, nullptr);
	assert(queueCount >= 1);

	m_queueFamilyProperties.resize(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueCount, m_queueFamilyProperties.data());
}


void GraphicsDevice::InitializeValidation()
{
#if USE_VALIDATION_LAYER
	VkInstance instance = m_instance->Get();

	vkCmdBeginDebugUtilsLabel = reinterpret_cast<PFN_vkCmdBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdBeginDebugUtilsLabelEXT"));
	vkCmdEndDebugUtilsLabel = reinterpret_cast<PFN_vkCmdEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdEndDebugUtilsLabelEXT"));
	vkCmdInsertDebugUtilsLabel = reinterpret_cast<PFN_vkCmdInsertDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkCmdInsertDebugUtilsLabelEXT"));
	vkCreateDebugUtilsMessenger = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));
	vkDestroyDebugUtilsMessenger = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));
	vkQueueBeginDebugUtilsLabel = reinterpret_cast<PFN_vkQueueBeginDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT"));
	vkQueueEndDebugUtilsLabel = reinterpret_cast<PFN_vkQueueEndDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT"));
	vkQueueInsertDebugUtilsLabel = reinterpret_cast<PFN_vkQueueInsertDebugUtilsLabelEXT>(vkGetInstanceProcAddr(instance, "vkQueueInsertDebugUtilsLabelEXT"));
	vkSetDebugUtilsObjectName = reinterpret_cast<PFN_vkSetDebugUtilsObjectNameEXT>(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT"));
	vkSetDebugUtilsObjectTag = reinterpret_cast<PFN_vkSetDebugUtilsObjectTagEXT>(vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectTagEXT"));
	vkSubmitDebugUtilsMessage = reinterpret_cast<PFN_vkSubmitDebugUtilsMessageEXT>(vkGetInstanceProcAddr(instance, "vkSubmitDebugUtilsMessageEXT"));

	assert(vkCmdBeginDebugUtilsLabel);
	assert(vkCmdEndDebugUtilsLabel);
	assert(vkCmdInsertDebugUtilsLabel);
	assert(vkCreateDebugUtilsMessenger);
	assert(vkDestroyDebugUtilsMessenger);
	assert(vkQueueBeginDebugUtilsLabel);
	assert(vkQueueEndDebugUtilsLabel);
	assert(vkQueueInsertDebugUtilsLabel);
	assert(vkSetDebugUtilsObjectName);
	assert(vkSetDebugUtilsObjectTag);
	assert(vkSubmitDebugUtilsMessage);

	g_debugMarkupAvailable = true;

#if ENABLE_VULKAN_VALIDATION
	VkDebugUtilsMessengerCreateInfoEXT createInfo = 
	{
		VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		nullptr,
		0,
		VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT,
		VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT,
		messageCallback,
		nullptr
	};

	VkDebugUtilsMessengerEXT messenger{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateDebugUtilsMessenger(instance, &createInfo, nullptr, &messenger));

	m_debugUtilsMessenger = new UVkDebugUtilsMessenger(m_instance.Get(), messenger);
#endif

#endif // USE_VALIDATION_LAYER
}


bool GraphicsDevice::IsExtensionSupported(const string& extension) const
{
	const char* c_str = extension.c_str();
	return find_if(
		m_deviceExtensions.begin(),
		m_deviceExtensions.end(),
		[c_str](auto const& e) { return strcmp(e.extensionName, c_str) == 0; }) != m_deviceExtensions.end();
}


void GraphicsDevice::GatherApplicationExtensions(bool optionalFeatures)
{
	auto& requestedExtensions = optionalFeatures ? m_optionalExtensions : m_requiredExtensions;
	auto& requestedFeatures = optionalFeatures ? g_optionalFeatures : g_requiredFeatures;

#if 0
	// VK_KHR_shader_float16_int8
	if (requestedFeatures.shaderFloat16 || requestedFeatures.shaderInt8)
		requestedExtensions.insert(VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME);
#endif

	// Add some required and optional extensions used by every Application
	if (optionalFeatures)
	{
		//m_requestedExtensions.insert(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
	}
	else
	{
		m_requestedExtensions.insert(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
	}
}


void GraphicsDevice::ValidateApplicationExtensions()
{
	vector<string> missingExtensions;

	// Check required extensions
	for (const auto& extName : m_requiredExtensions)
	{
		if (IsExtensionSupported(extName))
		{
			m_requestedExtensions.insert(extName);
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
	for (const auto& extName : m_optionalExtensions)
	{
		if (IsExtensionSupported(extName))
		{
			m_requestedExtensions.insert(extName);
		}
	}

	// Now, hook up pointers for all the per-extension device features
	void** pNextSupported = &m_supportedDeviceFeatures1_2.pNext;
	void** pNextEnabled = &m_enabledDeviceFeatures1_2.pNext;

	for (const auto& extName : m_requestedExtensions)
	{
#if 0
		if (extName == VK_KHR_SHADER_FLOAT16_INT8_EXTENSION_NAME)
		{
			*pNextSupported = &m_supportedExtendedFeatures.khrShaderFloat16Int8Features;
			pNextSupported = &m_supportedExtendedFeatures.khrShaderFloat16Int8Features.pNext;

			*pNextEnabled = &m_enabledExtendedFeatures.khrShaderFloat16Int8Features;
			pNextEnabled = &m_enabledExtendedFeatures.khrShaderFloat16Int8Features.pNext;
		}
#endif
	}
}


void GraphicsDevice::EnableFeatures(bool optionalFeatures)
{
	auto& requestedFeatures = optionalFeatures ? g_optionalFeatures : g_requiredFeatures;
	auto& enabledFeatures = const_cast<GraphicsFeatureSet&>(g_enabledFeatures);

	// Require timeline semaphores always
	if (!optionalFeatures)
	{
		TryEnableFeature(
			false,
			"Timeline Semaphore",
			m_supportedDeviceFeatures1_2.timelineSemaphore,
			m_enabledDeviceFeatures1_2.timelineSemaphore);
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
				m_supportedDeviceFeatures2.features.robustBufferAccess,
				m_enabledDeviceFeatures2.features.robustBufferAccess);
			break;

		case GraphicsFeature::FullDrawIndexUint32:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.fullDrawIndexUint32,
				m_enabledDeviceFeatures2.features.fullDrawIndexUint32);
			break;

		case GraphicsFeature::TextureCubeArray:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.imageCubeArray,
				m_enabledDeviceFeatures2.features.imageCubeArray);
			break;

		case GraphicsFeature::IndependentBlend:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.independentBlend,
				m_enabledDeviceFeatures2.features.independentBlend);
			break;

		case GraphicsFeature::GeometryShader:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.geometryShader,
				m_enabledDeviceFeatures2.features.geometryShader);
			break;

		case GraphicsFeature::TessellationShader:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.tessellationShader,
				m_enabledDeviceFeatures2.features.tessellationShader);
			break;

		case GraphicsFeature::SampleRateShading:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.sampleRateShading,
				m_enabledDeviceFeatures2.features.sampleRateShading);
			break;

		case GraphicsFeature::DualSrcBlend:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.dualSrcBlend,
				m_enabledDeviceFeatures2.features.dualSrcBlend);
			break;

		case GraphicsFeature::LogicOp:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.logicOp,
				m_enabledDeviceFeatures2.features.logicOp);
			break;

		case GraphicsFeature::DrawIndirectFirstInstance:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.drawIndirectFirstInstance,
				m_enabledDeviceFeatures2.features.drawIndirectFirstInstance);
			break;

		case GraphicsFeature::DepthClamp:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.depthClamp,
				m_enabledDeviceFeatures2.features.depthClamp);
			break;

		case GraphicsFeature::DepthBiasClamp:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.depthBiasClamp,
				m_enabledDeviceFeatures2.features.depthBiasClamp);
			break;

		case GraphicsFeature::FillModeNonSolid:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.fillModeNonSolid,
				m_enabledDeviceFeatures2.features.fillModeNonSolid);
			break;

		case GraphicsFeature::DepthBounds:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.depthBounds,
				m_enabledDeviceFeatures2.features.depthBounds);
			break;

		case GraphicsFeature::WideLines:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.wideLines,
				m_enabledDeviceFeatures2.features.wideLines);
			break;

		case GraphicsFeature::LargePoints:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.largePoints,
				m_enabledDeviceFeatures2.features.largePoints);
			break;

		case GraphicsFeature::AlphaToOne:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.alphaToOne,
				m_enabledDeviceFeatures2.features.alphaToOne);
			break;

		case GraphicsFeature::MultiViewport:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.multiViewport,
				m_enabledDeviceFeatures2.features.multiViewport);
			break;

		case GraphicsFeature::SamplerAnisotropy:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.samplerAnisotropy,
				m_enabledDeviceFeatures2.features.samplerAnisotropy);
			break;

		case GraphicsFeature::TextureCompressionETC2:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.textureCompressionETC2,
				m_enabledDeviceFeatures2.features.textureCompressionETC2);
			break;

		case GraphicsFeature::TextureCompressionASTC_LDR:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.textureCompressionASTC_LDR,
				m_enabledDeviceFeatures2.features.textureCompressionASTC_LDR);
			break;

		case GraphicsFeature::TextureCompressionBC:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.textureCompressionBC,
				m_enabledDeviceFeatures2.features.textureCompressionBC);
			break;

		case GraphicsFeature::OcclusionQueryPrecise:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.occlusionQueryPrecise,
				m_enabledDeviceFeatures2.features.occlusionQueryPrecise);
			break;

		case GraphicsFeature::PipelineStatisticsQuery:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.pipelineStatisticsQuery,
				m_enabledDeviceFeatures2.features.pipelineStatisticsQuery);
			break;

		case GraphicsFeature::VertexPipelineStoresAndAtomics:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.vertexPipelineStoresAndAtomics,
				m_enabledDeviceFeatures2.features.vertexPipelineStoresAndAtomics);
			break;

		case GraphicsFeature::PixelShaderStoresAndAtomics:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.fragmentStoresAndAtomics,
				m_enabledDeviceFeatures2.features.fragmentStoresAndAtomics);
			break;

		case GraphicsFeature::ShaderTessellationAndGeometryPointSize:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.shaderTessellationAndGeometryPointSize,
				m_enabledDeviceFeatures2.features.shaderTessellationAndGeometryPointSize);
			break;

		case GraphicsFeature::ShaderTextureGatherExtended:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.shaderImageGatherExtended,
				m_enabledDeviceFeatures2.features.shaderImageGatherExtended);
			break;
		case GraphicsFeature::ShaderUAVExtendedFormats:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.shaderStorageImageExtendedFormats,
				m_enabledDeviceFeatures2.features.shaderStorageImageExtendedFormats);
			break;

		case GraphicsFeature::ShaderUAVMultisample:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.shaderStorageImageMultisample,
				m_enabledDeviceFeatures2.features.shaderStorageImageMultisample);
			break;

		case GraphicsFeature::ShaderClipDistance:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.shaderClipDistance,
				m_enabledDeviceFeatures2.features.shaderClipDistance);
			break;
		case GraphicsFeature::ShaderCullDistance:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.shaderCullDistance,
				m_enabledDeviceFeatures2.features.shaderCullDistance);
			break;
		case GraphicsFeature::ShaderFloat64:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.shaderFloat64,
				m_enabledDeviceFeatures2.features.shaderFloat64);
			break;
		case GraphicsFeature::ShaderFloat16:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures1_2.shaderFloat16,
				m_enabledDeviceFeatures1_2.shaderFloat16);
			break;
		case GraphicsFeature::ShaderInt64:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.shaderInt64,
				m_enabledDeviceFeatures2.features.shaderInt64);
			break;
		case GraphicsFeature::ShaderInt16:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.shaderInt16,
				m_enabledDeviceFeatures2.features.shaderInt16);
			break;
		case GraphicsFeature::ShaderInt8:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures1_2.shaderInt8,
				m_enabledDeviceFeatures1_2.shaderInt8);
			break;

		case GraphicsFeature::VariableMultisampleRate:
			enabledFeature = TryEnableFeature(
				optionalFeatures,
				name,
				m_supportedDeviceFeatures2.features.variableMultisampleRate,
				m_enabledDeviceFeatures2.features.variableMultisampleRate);
			break;
		}
	}
}


bool GraphicsDevice::TryEnableFeature(bool optional, const string& name, const VkBool32& supportedFeature, VkBool32& enabledFeature)
{
	bool supported = VK_TRUE == supportedFeature;
	if (!optional && !supported)
	{
		m_unsupportedRequiredFeatures.push_back(name);
		ExitFatal(
			"Required Feature Not Supported",
			"This Application requires " + name + ", which is unavailable.  You may need to update your GPU or graphics driver");
	}
	enabledFeature = supportedFeature;
	return supported;
}


uint32_t GraphicsDevice::GetQueueFamilyIndex(VkQueueFlags queueFlags) const
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


bool GraphicsDevice::GetSurfaceSupport(uint32_t index, VkSurfaceKHR surface) const
{
	VkBool32 supported = VK_FALSE;
	ThrowIfFailed(vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice->Get(), index, surface, &supported));
	return supported == VK_TRUE;
}


vector<uint32_t> GraphicsDevice::GetGraphicsPresentQueueFamilyIndices(VkSurfaceKHR surface) const
{
	vector<uint32_t> indices;
	for (size_t i = 0; i < m_queueFamilyProperties.size(); ++i)
	{
		if ((m_queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && GetSurfaceSupport((uint32_t)i, surface))
		{
			indices.push_back((uint32_t)i);
		}
	}
	return indices;
}


vector<uint32_t> GraphicsDevice::GetQueueFamilyIndices(VkQueueFlags queueFlags) const
{
	vector<uint32_t> indices;
	for (size_t i = 0; i < m_queueFamilyProperties.size(); ++i)
	{
		if ((m_queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
		{
			indices.push_back((uint32_t)i);
		}
	}
	return indices;
}


VkDevice Kodiak::GetDevice()
{
	return g_device->Get();
}


VmaAllocator Kodiak::GetAllocator()
{
	return g_graphicsDevice->GetAllocator();
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
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_INSTANCE, name.c_str());
}


void Kodiak::SetDebugName(VkPhysicalDevice obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_PHYSICAL_DEVICE, name.c_str());
}


void Kodiak::SetDebugName(VkDevice obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_DEVICE, name.c_str());
}


void Kodiak::SetDebugName(VkQueue obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_QUEUE, name.c_str());
}


void Kodiak::SetDebugName(VkSemaphore obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_SEMAPHORE, name.c_str());
}


void Kodiak::SetDebugName(VkCommandBuffer obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_COMMAND_BUFFER, name.c_str());
}


void Kodiak::SetDebugName(VkFence obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_FENCE, name.c_str());
}


void Kodiak::SetDebugName(VkDeviceMemory obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_DEVICE_MEMORY, name.c_str());
}


void Kodiak::SetDebugName(VkBuffer obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_BUFFER, name.c_str());
}


void Kodiak::SetDebugName(VkImage obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_IMAGE, name.c_str());
}


void Kodiak::SetDebugName(VkEvent obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_EVENT, name.c_str());
}


void Kodiak::SetDebugName(VkQueryPool obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_QUERY_POOL, name.c_str());
}


void Kodiak::SetDebugName(VkBufferView obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_BUFFER_VIEW, name.c_str());
}


void Kodiak::SetDebugName(VkImageView obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_IMAGE_VIEW, name.c_str());
}


void Kodiak::SetDebugName(VkShaderModule obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_SHADER_MODULE, name.c_str());
}


void Kodiak::SetDebugName(VkPipelineCache obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_PIPELINE_CACHE, name.c_str());
}


void Kodiak::SetDebugName(VkPipelineLayout obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_PIPELINE_LAYOUT, name.c_str());
}


void Kodiak::SetDebugName(VkRenderPass obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_RENDER_PASS, name.c_str());
}


void Kodiak::SetDebugName(VkPipeline obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_PIPELINE, name.c_str());
}


void Kodiak::SetDebugName(VkDescriptorSetLayout obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT, name.c_str());
}


void Kodiak::SetDebugName(VkSampler obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_SAMPLER, name.c_str());
}


void Kodiak::SetDebugName(VkDescriptorPool obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_DESCRIPTOR_POOL, name.c_str());
}


void Kodiak::SetDebugName(VkDescriptorSet obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_DESCRIPTOR_SET, name.c_str());
}


void Kodiak::SetDebugName(VkFramebuffer obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_FRAMEBUFFER, name.c_str());
}


void Kodiak::SetDebugName(VkCommandPool obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_COMMAND_POOL, name.c_str());
}


void Kodiak::SetDebugName(VkSurfaceKHR obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_SURFACE_KHR, name.c_str());
}


void Kodiak::SetDebugName(VkSwapchainKHR obj, const string& name)
{
	::SetDebugName(reinterpret_cast<uint64_t>(obj), VK_OBJECT_TYPE_SWAPCHAIN_KHR, name.c_str());
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

#endif