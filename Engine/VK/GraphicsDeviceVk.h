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
public:
	GraphicsDevice();
	~GraphicsDevice();

	void Initialize(const std::string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height);
	void Destroy();

	void PrepareFrame();
	void SubmitFrame();

private:
	void CreateVulkanInstance();
	void SelectPhysicalDevice();
	void CreateLogicalDevice();
	void CreateSynchronizationPrimitives();

	void InitializeDebugging(VkDebugReportFlagsEXT flags, VkDebugReportCallbackEXT callBack);
	void FreeDebugCallback();

	uint32_t GetQueueFamilyIndex(VkQueueFlagBits queueFlags);
	bool IsExtensionSupported(const std::string& name);

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
};

} // namespace Kodiak