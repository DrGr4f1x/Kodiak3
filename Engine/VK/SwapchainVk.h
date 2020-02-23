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

namespace Kodiak
{

// Forward declarations
class Image;
class LogicalDevice;
class Surface;


class Swapchain : public Reference<LogicalDevice>, public NonCopyable
{
public:
	Swapchain(
		const std::shared_ptr<LogicalDevice>& logicalDevice,
		const std::shared_ptr<Surface>& surface,
		uint32_t minImageCount,
		VkFormat imageFormat,
		const VkExtent2D& imageExtent,
		uint32_t imageArrayLayers,
		VkImageUsageFlags imageUsage,
		VkSharingMode imageSharingMode,
		const std::vector<uint32_t>& queueFamilyIndices,
		VkSurfaceTransformFlagBitsKHR preTransform,
		VkCompositeAlphaFlagBitsKHR compositeAlpha,
		VkPresentModeKHR presentMode,
		bool clipped,
		const std::shared_ptr<Swapchain>& oldSwapchain);
	virtual ~Swapchain();

	const std::vector<std::shared_ptr<Image>>& GetImages() const { return m_images; }

	operator VkSwapchainKHR();

protected:
	void Initialize(
		const std::shared_ptr<Surface>& surface,
		uint32_t minImageCount,
		VkFormat imageFormat,
		const VkExtent2D& imageExtent,
		uint32_t imageArrayLayers,
		VkImageUsageFlags imageUsage,
		VkSharingMode imageSharingMode,
		const std::vector<uint32_t>& queueFamilyIndices,
		VkSurfaceTransformFlagBitsKHR preTransform,
		VkCompositeAlphaFlagBitsKHR compositeAlpha,
		VkPresentModeKHR presentMode,
		bool clipped,
		const std::shared_ptr<Swapchain>& oldSwapchain);

private:
	VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
	std::vector<std::shared_ptr<Image>> m_images;
};


inline Swapchain::operator VkSwapchainKHR()
{
	return m_swapchain;
}

} // namespace Kodiak