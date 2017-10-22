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

typedef struct _SwapChainBuffers 
{
	VkImage image;
	VkImageView view;
} SwapChainBuffer;


class SwapChain
{
public:
	void Connect(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
	void InitSurface(HINSTANCE hInstance, HWND hWnd);

	void Create(uint32_t* width, uint32_t* height, bool vsync = false);
	void Destroy();

	VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, uint32_t* imageIndex);
	VkResult QueuePresent(VkQueue queue, uint32_t imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

	VkFormat GetColorFormat() const { return m_colorFormat; }
	uint32_t GetImageCount() const { return m_imageCount; }

	// TODO: remove this hack
	VkImageView GetImageView(uint32_t index) const { return m_buffers[index].view; }

private:
	VkInstance m_instance{ VK_NULL_HANDLE };
	VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };
	VkDevice m_device{ VK_NULL_HANDLE };

	VkSurfaceKHR m_surface{ VK_NULL_HANDLE };

	uint32_t m_queueNodeIndex{ UINT32_MAX };  // Index of the deteced graphics and presenting device queue

	VkFormat m_colorFormat{ VK_FORMAT_UNDEFINED };
	VkColorSpaceKHR m_colorSpace;
	
	VkSwapchainKHR m_swapChain{ VK_NULL_HANDLE };  // Handle to the current swap chain, required for recreation

	uint32_t m_imageCount{ 0 };
	std::vector<VkImage> m_images;
	std::vector<SwapChainBuffer> m_buffers;
};

} // namespace Kodiak