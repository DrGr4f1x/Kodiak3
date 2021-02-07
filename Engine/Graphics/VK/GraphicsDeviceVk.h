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

#include "Graphics\ColorBuffer.h"
#include "Graphics\GraphicsFeatures.h"


namespace Kodiak
{

class GraphicsDevice
{
public:
	GraphicsDevice();
	~GraphicsDevice();

	void Initialize(const std::string& appName, HINSTANCE hInstance, HWND hWnd, uint32_t width, uint32_t height, Format colorFormat, Format depthFormat);
	void Destroy();

	void SubmitFrame();

	void WaitForGpuIdle();

	// Create methods
	std::shared_ptr<SemaphoreRef> CreateSemaphore(VkSemaphoreType semaphoreType) const;
	std::shared_ptr<AllocatorRef> CreateAllocator() const;

	Format GetColorFormat() const { return m_colorFormat; }
	Format GetDepthFormat() const { return m_depthFormat; }

	ColorBufferPtr GetBackBuffer(uint32_t index) const;
	uint32_t GetCurrentBuffer() const { return m_currentBuffer; }

	uint32_t GetFrameNumber() const { return m_frameNumber; }

	const std::string& GetDeviceName() const { return m_deviceName; }

	void ReleaseResource(PlatformHandle handle);

	uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const;
	VkFormatProperties GetFormatProperties(Format format);
	uint32_t GetQueueFamilyIndex(CommandListType type) const;

private:
	void ReleaseDeferredResources();

	void InitializeInternal();
	void Present();

	void CreateInstance();
	void SelectPhysicalDevice();
	void CreateLogicalDevice();
	void InitSurface();
	void CreateSwapChain();

	uint32_t AcquireNextImage();
	void WaitForImageAcquisition(VkQueue queue);
	void UnblockPresent(VkQueue queue, VkSemaphore timelineSemaphore, uint64_t fenceWaitValue);

	void GetPhysicalDeviceProperties();
	void InitializeValidation();

	bool IsExtensionSupported(const std::string& extension) const;
	void GatherApplicationExtensions(bool optionalFeatures);
	void ValidateApplicationExtensions();
	void EnableFeatures(bool optionalFeatures);
	bool TryEnableFeature(bool optional, const std::string& name, const VkBool32& supportedFeature, VkBool32& enabledFeature);

	uint32_t GetQueueFamilyIndex(VkQueueFlags queueFlags) const;
	bool GetSurfaceSupport(uint32_t index, VkSurfaceKHR surface) const;
	std::vector<uint32_t> GetGraphicsPresentQueueFamilyIndices(VkSurfaceKHR surface) const;
	std::vector<uint32_t> GetQueueFamilyIndices(VkQueueFlags queueFlags) const;

private:
	std::string m_appName;

	HINSTANCE m_hinst{ 0 };
	HWND m_hwnd{ 0 };

	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	bool m_vsync{ false };
	Format m_colorFormat{ Format::Unknown };
	Format m_depthFormat{ Format::Unknown };

	std::string m_deviceName{ "Unknown" };

	std::array<ColorBufferPtr, NumSwapChainBuffers> m_swapChainBuffers;
	uint32_t m_currentBuffer{ 0 };

	uint32_t m_frameNumber{ 0 };

	// Deferred resource release
	struct DeferredReleaseResource
	{
		uint64_t fenceValue;
		PlatformHandle resourceHandle;
	};
	std::list<DeferredReleaseResource> m_deferredResources;

	// Platform-specific implementation
	struct PlatformData;
	PlatformData* m_platformData{ nullptr };

	// Vulkan members (ref-counted)
	std::shared_ptr<InstanceRef> m_instance;
	std::shared_ptr<PhysicalDeviceRef> m_physicalDevice;
	std::shared_ptr<DeviceRef> m_device;
	std::shared_ptr<SemaphoreRef> m_imageAcquireSemaphore;
	std::shared_ptr<SemaphoreRef> m_presentSemaphore;
	std::shared_ptr<AllocatorRef> m_allocator;
	std::shared_ptr<SurfaceRef> m_surface;
	std::shared_ptr<SwapchainRef> m_swapchain;
	std::shared_ptr<DebugUtilsMessengerRef> m_debugUtilsMessenger;

	// Swapchain images
	std::vector<std::shared_ptr<ImageRef>> m_swapchainImages;

	// Physical devices, properties, and extensions

	VkPhysicalDeviceProperties				m_physicalDeviceProperties;
	VkPhysicalDeviceMemoryProperties		m_physicalDeviceMemoryProperties;

	std::vector<VkExtensionProperties>		m_deviceExtensions;

	// Required, optional, and enabled extensions
	std::set<std::string> m_supportedExtensions;
	std::set<std::string> m_requiredExtensions;
	std::set<std::string> m_optionalExtensions;
	std::set<std::string> m_requestedExtensions;
	std::vector<std::string> m_requestedLayers;

	std::vector<std::string> m_unsupportedRequiredFeatures;

	// Base features
	VkPhysicalDeviceFeatures2			m_supportedDeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, nullptr };
	VkPhysicalDeviceFeatures2			m_enabledDeviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, nullptr };
	VkPhysicalDeviceVulkan12Features	m_supportedDeviceFeatures1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr };
	VkPhysicalDeviceVulkan12Features	m_enabledDeviceFeatures1_2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, nullptr };

	// Extended features
#if 0
	struct
	{
		VkPhysicalDeviceShaderFloat16Int8FeaturesKHR khrShaderFloat16Int8Features{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_SHADER_FLOAT16_INT8_FEATURES_KHR, nullptr };
	} m_supportedExtendedFeatures, m_enabledExtendedFeatures;
#endif

	// Queues and queue families
	std::vector<VkQueueFamilyProperties> m_queueFamilyProperties;
	struct
	{
		uint32_t graphics;
		uint32_t compute;
		uint32_t transfer;
	} m_queueFamilyIndices;

	uint32_t m_presentQueueNodeIndex{ 0 };

	VkColorSpaceKHR m_colorSpace;
};

extern GraphicsDevice* g_graphicsDevice;


const DeviceHandle GetDevice();


uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);
VkFormatProperties GetFormatProperties(Format format);

} // namespace Kodiak