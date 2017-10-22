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

class SwapChain;

class GraphicsDevice
{
	friend uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound);

public:
	GraphicsDevice();
	~GraphicsDevice();

	void Initialize(const std::string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height);
	void Destroy();

	void PrepareFrame();
	void SubmitFrame();

	SwapChain* GetSwapChain() { return m_swapChain.get(); }

	VkFormat GetDepthFormat() const { return m_depthFormat; }
	uint32_t GetCurrentBuffer() const { return m_currentBuffer; }

	// TODO: Hacks
	VkSemaphore GetPresentCompleteSemaphore() const { return m_semaphores.presentComplete; }
	VkSemaphore GetRenderCompleteSemaphore() const { return m_semaphores.renderComplete; }

private:
	void CreateVulkanInstance();
	void SelectPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSynchronizationPrimitives();

	void InitializeDebugging(VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack);
	void InitializeDebugMarkup();
	void FreeDebugCallback();

	uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags);
	bool IsExtensionSupported(const std::string& name);

	void FindBestDepthFormat();

private:
	bool m_initialized{ false };

	std::string m_appName;

	HINSTANCE m_hinst{ 0 };
	HWND m_hwnd{ 0 };

	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_destWidth{ 0 };
	uint32_t m_destHeight{ 0 };

	VkInstance m_instance{ VK_NULL_HANDLE };

	uint32_t m_gpuIndex{ 0 };
	VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };

	VkPhysicalDeviceProperties m_physicalDeviceProperties{};
	VkPhysicalDeviceFeatures m_physicalDeviceSupportedFeatures{};
	VkPhysicalDeviceFeatures m_physicalDeviceEnabledFeatures{};
	VkPhysicalDeviceMemoryProperties m_physicalDeviceMemoryProperties{};

	struct
	{
		uint32_t graphics;
		uint32_t compute;
		uint32_t transfer;
	} m_queueFamilyIndices;

	std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
	VkQueue m_graphicsQueue{ VK_NULL_HANDLE };

	VkDevice m_device{ VK_NULL_HANDLE };

	// TODO: handle this differently
	std::vector<std::string> m_supportedExtensions;
	std::vector<const char*> m_enabledExtensions;

	std::unique_ptr<SwapChain> m_swapChain;

	// Synchronization semaphores
	struct
	{
		// Swap chain image presentation
		VkSemaphore presentComplete{ VK_NULL_HANDLE };
		// Command buffer submission and execution
		VkSemaphore renderComplete{ VK_NULL_HANDLE };
		// Text overlay submission and execution
		VkSemaphore textOverlayComplete{ VK_NULL_HANDLE };
	} m_semaphores;

	// Contains command buffers and semaphores to be presented to the queue
	VkSubmitInfo m_submitInfo;
	// Pipeline stages used to wait at for graphics queue submissions
	VkPipelineStageFlags m_submitPipelineStages{ VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };

	uint32_t m_currentBuffer{ 0 };

	VkFormat m_depthFormat{ VK_FORMAT_UNDEFINED };
};


// Utility methods and accessors
VkDevice GetDevice();
uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);

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