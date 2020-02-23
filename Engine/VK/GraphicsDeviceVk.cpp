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

#include "AllocatorVk.h"
#include "CommandContextVk.h"
#include "CommandListManagerVk.h"
#include "DebugVk.h"
#include "DescriptorHeapVk.h"
#include "ImageVk.h"
#include "InstanceVk.h"
#include "LogicalDeviceVk.h"
#include "PhysicalDeviceVk.h"
#include "RootSignatureVk.h"
#include "SemaphoreVk.h"
#include "SurfaceVk.h"
#include "SwapchainVk.h"

#include <iostream>
#include <sstream>


using namespace Kodiak;
using namespace Utility;
using namespace std;


namespace
{

// TODO - Delete me
DeviceHandle g_device{ VK_NULL_HANDLE };

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
		enabledDeviceFeatures2.pNext = &enabledDeviceFeatures1_2;
	}


	~PlatformData() = default;


	void SelectPhysicalDevice()
	{
		// GPU selection
		physicalDevice = instance->GetPhysicalDevice(0);

		// Initialize debug markup
		instance->InitializeDebugMarkup(physicalDevice);

		// Get available physical device features
		supportedDeviceFeatures2 = physicalDevice->GetDeviceFeatures2();
		supportedDeviceFeatures1_2 = physicalDevice->GetDeviceFeatures1_2();

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
		queueFamilyIndices.graphics = ::GetQueueFamilyIndex(physicalDevice, VK_QUEUE_GRAPHICS_BIT);
		VkDeviceQueueCreateInfo queueInfo{};
		queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
		queueInfo.queueCount = 1;
		queueInfo.pQueuePriorities = &defaultQueuePriority;
		queueCreateInfos.push_back(queueInfo);

		// Dedicated compute queue
		queueFamilyIndices.compute = ::GetQueueFamilyIndex(physicalDevice, VK_QUEUE_COMPUTE_BIT);
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
		queueFamilyIndices.transfer = ::GetQueueFamilyIndex(physicalDevice, VK_QUEUE_TRANSFER_BIT);
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

		vector<string> extensions(requestedExtensions.begin(), requestedExtensions.end());
		device = physicalDevice->CreateLogicalDevice(queueCreateInfos, requestedLayers, extensions, enabledDeviceFeatures2);

		// TODO - Delete me
		g_device = GetLogicalDevice();

		// Create allocator
		allocator = Allocator::Create(instance, physicalDevice, device);

		// Create semaphores
		presentSemaphore = device->CreateBinarySemaphore();
		imageAcquireSemaphore = device->CreateBinarySemaphore();
	}


	void InitSurface(HINSTANCE hInstance, HWND hWnd)
	{
		// Create surface
		surface = instance->CreateSurface(hInstance, hWnd);

		// Find a graphics queue that supports present
		const auto& queueFamilyIndices = GetGraphicsPresentQueueFamilyIndices(physicalDevice, surface);
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
		swapchain = device->CreateSwapchain(
			surface,
			desiredNumberOfSwapchainImages,
			vkFormat,
			swapchainExtent,
			1u, // imageArrayLayers
			usage,
			VK_SHARING_MODE_EXCLUSIVE,
			{}, // queueFamilyIndices
			preTransform,
			compositeAlpha,
			swapchainPresentMode,
			true, // clipped
			oldSwapchain);
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
			if (physicalDevice->IsExtensionSupported(extName))
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
			if (physicalDevice->IsExtensionSupported(extName))
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
	shared_ptr<Instance> instance;
	shared_ptr<PhysicalDevice> physicalDevice;
	shared_ptr<LogicalDevice> device;
	shared_ptr<Allocator> allocator;
	shared_ptr<Surface> surface;
	shared_ptr<Swapchain> swapchain;
	shared_ptr<DebugReportCallback> debugReportCallback;
	shared_ptr<Semaphore> imageAcquireSemaphore;
	shared_ptr<Semaphore> presentSemaphore;

	vector<string> unsupportedRequiredFeatures;

	struct
	{
		uint32_t graphics;
		uint32_t compute;
		uint32_t transfer;
	} queueFamilyIndices;

	// Required, optional, and enabled extensions
	set<string> supportedExtensions;
	set<string> requiredExtensions;
	set<string> optionalExtensions;
	set<string> requestedExtensions;
	vector<string> requestedLayers;

	uint32_t presentQueueNodeIndex{ 0 };
	VkColorSpaceKHR colorSpace;

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
	return m_platformData->physicalDevice->GetMemoryTypeIndex(typeBits, properties, memTypeFound);
}


void GraphicsDevice::PlatformCreate()
{
	m_platformData = new PlatformData;

	m_platformData->instance = Instance::Create(m_appName);

#if ENABLE_VULKAN_VALIDATION
	// Enable validation for debugging

	// The report flags determine what type of messages for the layers will be displayed
	// For validating (debugging) an application the error and warning bits should suffice
	VkDebugReportFlagsEXT debugReportFlags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	// Additional flags include performance info, loader and layer debug messages, etc.

	m_platformData->debugReportCallback = m_platformData->instance->CreateDebugReportCallback(debugReportFlags);
#endif

	m_platformData->SelectPhysicalDevice();
	m_deviceName = m_platformData->physicalDevice->GetDeviceProperties().deviceName;

	m_platformData->CreateLogicalDevice();

	m_platformData->InitSurface(m_hinst, m_hwnd);
	m_platformData->CreateSwapChain(&m_width, &m_height, false /* vsync */);

	// Get ColorBuffers for swap chain images
	for (uint32_t i = 0; i < NumSwapChainBuffers; ++i)
	{
		m_swapChainBuffers[i] = make_shared<ColorBuffer>();
		VkImage image = *m_platformData->swapchain->GetImages()[i];
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