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

#include "SwapChainVk.h"

#include "Utility.h"

using namespace Kodiak;
using namespace std;


void SwapChain::Connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
	m_instance = instance;
	m_physicalDevice = physicalDevice;
	m_device = device;
}


void SwapChain::InitSurface(HINSTANCE hInstance, HWND hWnd)
{
	VkResult err = VK_SUCCESS;

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = hInstance;
	surfaceCreateInfo.hwnd = hWnd;
	err = vkCreateWin32SurfaceKHR(m_instance, &surfaceCreateInfo, nullptr, &m_surface);

	if (err != VK_SUCCESS) 
	{
		Utility::ExitFatal("Could not create surface!", "Fatal error");
	}

	// Get available queue family properties
	uint32_t queueCount;
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueCount, nullptr);
	assert(queueCount >= 1);

	vector<VkQueueFamilyProperties> queueProps(queueCount);
	vkGetPhysicalDeviceQueueFamilyProperties(m_physicalDevice, &queueCount, queueProps.data());

	// Iterate over each queue to learn whether it supports presenting:
	// Find a queue with present support
	// Will be used to present the swap chain images to the windowing system
	vector<VkBool32> supportsPresent(queueCount);
	for (uint32_t i = 0; i < queueCount; i++)
	{
		vkGetPhysicalDeviceSurfaceSupportKHR(m_physicalDevice, i, m_surface, &supportsPresent[i]);
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

	m_queueNodeIndex = graphicsQueueNodeIndex;

	// Get list of supported surface formats
	uint32_t formatCount;
	ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, nullptr));
	assert(formatCount > 0);

	vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	ThrowIfFailed(vkGetPhysicalDeviceSurfaceFormatsKHR(m_physicalDevice, m_surface, &formatCount, surfaceFormats.data()));

	// If the surface format list only includes one entry with VK_FORMAT_UNDEFINED,
	// there is no preferered format, so we assume VK_FORMAT_B8G8R8A8_UNORM
	if ((formatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED))
	{
		m_colorFormat = VK_FORMAT_B8G8R8A8_UNORM;
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
				m_colorFormat = surfaceFormat.format;
				m_colorSpace = surfaceFormat.colorSpace;
				found_B8G8R8A8_UNORM = true;
				break;
			}
		}

		// in case VK_FORMAT_B8G8R8A8_UNORM is not available
		// select the first available color format
		if (!found_B8G8R8A8_UNORM)
		{
			m_colorFormat = surfaceFormats[0].format;
			m_colorSpace = surfaceFormats[0].colorSpace;
		}
	}
}


void SwapChain::Create(uint32_t* width, uint32_t* height, bool vsync)
{
	VkSwapchainKHR oldSwapchain = m_swapChain;

	// Get physical device surface properties and formats
	VkSurfaceCapabilitiesKHR surfCaps;
	ThrowIfFailed(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_physicalDevice, m_surface, &surfCaps));

	// Get available present modes
	uint32_t presentModeCount;
	ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, nullptr));
	assert(presentModeCount > 0);

	vector<VkPresentModeKHR> presentModes(presentModeCount);
	ThrowIfFailed(vkGetPhysicalDeviceSurfacePresentModesKHR(m_physicalDevice, m_surface, &presentModeCount, presentModes.data()));

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

	VkSwapchainCreateInfoKHR swapchainCI = {};
	swapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCI.pNext = nullptr;
	swapchainCI.surface = m_surface;
	swapchainCI.minImageCount = desiredNumberOfSwapchainImages;
	swapchainCI.imageFormat = m_colorFormat;
	swapchainCI.imageColorSpace = m_colorSpace;
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
	vkGetPhysicalDeviceFormatProperties(m_physicalDevice, m_colorFormat, &formatProps);
	if (formatProps.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT) 
	{
		swapchainCI.imageUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	ThrowIfFailed(vkCreateSwapchainKHR(m_device, &swapchainCI, nullptr, &m_swapChain));

	// If an existing swap chain is re-created, destroy the old swap chain
	// This also cleans up all the presentable images
	if (oldSwapchain != VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < m_imageCount; ++i)
		{
			vkDestroyImageView(m_device, m_buffers[i].view, nullptr);
		}
		vkDestroySwapchainKHR(m_device, oldSwapchain, nullptr);
	}
	ThrowIfFailed(vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, nullptr));

	// Get the swap chain images
	m_images.resize(m_imageCount);
	ThrowIfFailed(vkGetSwapchainImagesKHR(m_device, m_swapChain, &m_imageCount, m_images.data()));

	// Get the swap chain buffers containing the image and imageview
	m_buffers.resize(m_imageCount);
	for (uint32_t i = 0; i < m_imageCount; ++i)
	{
		VkImageViewCreateInfo colorAttachmentView = {};
		colorAttachmentView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		colorAttachmentView.pNext = nullptr;
		colorAttachmentView.format = m_colorFormat;
		colorAttachmentView.components = 
		{
			VK_COMPONENT_SWIZZLE_R,
			VK_COMPONENT_SWIZZLE_G,
			VK_COMPONENT_SWIZZLE_B,
			VK_COMPONENT_SWIZZLE_A
		};
		colorAttachmentView.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		colorAttachmentView.subresourceRange.baseMipLevel = 0;
		colorAttachmentView.subresourceRange.levelCount = 1;
		colorAttachmentView.subresourceRange.baseArrayLayer = 0;
		colorAttachmentView.subresourceRange.layerCount = 1;
		colorAttachmentView.viewType = VK_IMAGE_VIEW_TYPE_2D;
		colorAttachmentView.flags = 0;

		m_buffers[i].image = m_images[i];

		colorAttachmentView.image = m_buffers[i].image;

		ThrowIfFailed(vkCreateImageView(m_device, &colorAttachmentView, nullptr, &m_buffers[i].view));
	}
}


void SwapChain::Destroy()
{
	if (m_swapChain != VK_NULL_HANDLE)
	{
		for (uint32_t i = 0; i < m_imageCount; ++i)
		{
			vkDestroyImageView(m_device, m_buffers[i].view, nullptr);
		}
	}

	if (m_surface != VK_NULL_HANDLE)
	{
		vkDestroySwapchainKHR(m_device, m_swapChain, nullptr);
		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}
	m_surface = VK_NULL_HANDLE;
	m_swapChain = VK_NULL_HANDLE;
}