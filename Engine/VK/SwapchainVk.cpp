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

#include "SwapchainVk.h"

#include "ImageVk.h"
#include "LogicalDeviceVk.h"
#include "SurfaceVk.h"


using namespace Kodiak;
using namespace std;


Swapchain::Swapchain(
	const shared_ptr<LogicalDevice>& logicalDevice,
	const shared_ptr<Surface>& surface,
	uint32_t minImageCount,
	VkFormat imageFormat,
	const VkExtent2D& imageExtent,
	uint32_t imageArrayLayers,
	VkImageUsageFlags imageUsage,
	VkSharingMode imageSharingMode,
	const vector<uint32_t>& queueFamilyIndices,
	VkSurfaceTransformFlagBitsKHR preTransform,
	VkCompositeAlphaFlagBitsKHR compositeAlpha,
	VkPresentModeKHR presentMode,
	bool clipped,
	const shared_ptr<Swapchain>& oldSwapchain)
	: Reference(logicalDevice)
{
	Initialize(
		surface,
		minImageCount,
		imageFormat,
		imageExtent,
		imageArrayLayers,
		imageUsage,
		imageSharingMode,
		queueFamilyIndices,
		preTransform,
		compositeAlpha,
		presentMode,
		clipped,
		oldSwapchain);
}


Swapchain::~Swapchain()
{
	vkDestroySwapchainKHR(*Get<LogicalDevice>(), m_swapchain, nullptr);
	m_swapchain = VK_NULL_HANDLE;
}


void Swapchain::Initialize(
	const shared_ptr<Surface>& surface,
	uint32_t minImageCount,
	VkFormat imageFormat,
	const VkExtent2D& imageExtent,
	uint32_t imageArrayLayers,
	VkImageUsageFlags imageUsage,
	VkSharingMode imageSharingMode,
	const vector<uint32_t>& queueFamilyIndices,
	VkSurfaceTransformFlagBitsKHR preTransform,
	VkCompositeAlphaFlagBitsKHR compositeAlpha,
	VkPresentModeKHR presentMode,
	bool clipped,
	const shared_ptr<Swapchain>& oldSwapchain)
{
	static_assert(VK_COLOR_SPACE_RANGE_SIZE_KHR == 1, "Add argument 'colorSpace' when this assertion fires");

	VkSwapchainKHR oldsc = VK_NULL_HANDLE;
	if (oldSwapchain)
	{
		oldsc = *oldSwapchain;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.pNext = nullptr;
	createInfo.surface = *surface;
	createInfo.minImageCount = minImageCount;
	createInfo.imageFormat = imageFormat;
	createInfo.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
	createInfo.imageExtent = imageExtent;
	createInfo.imageUsage = imageUsage;
	createInfo.preTransform = preTransform;
	createInfo.imageArrayLayers = imageArrayLayers;
	createInfo.imageSharingMode = imageSharingMode;
	createInfo.queueFamilyIndexCount = (uint32_t)queueFamilyIndices.size();
	createInfo.pQueueFamilyIndices = queueFamilyIndices.data();
	createInfo.presentMode = presentMode;
	createInfo.oldSwapchain = oldsc;
	// Setting clipped to VK_TRUE allows the implementation to discard rendering outside of the surface area
	createInfo.clipped = clipped ? VK_TRUE : VK_FALSE;
	createInfo.compositeAlpha = compositeAlpha;

	VkDevice device = *Get<LogicalDevice>();

	ThrowIfFailed(vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapchain));

	// Count actual swapchain images
	uint32_t imageCount{ 0 };
	ThrowIfFailed(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, nullptr));

	// Get the swap chain images
	vector<VkImage> images(imageCount);
	ThrowIfFailed(vkGetSwapchainImagesKHR(device, m_swapchain, &imageCount, images.data()));

	m_images.reserve(imageCount);
	for (auto image : images)
	{
		m_images.push_back(make_shared<Image>(Get<LogicalDevice>(), image));
	}
}