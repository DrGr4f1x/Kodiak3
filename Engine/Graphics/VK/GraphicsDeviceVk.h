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
#include "Graphics\DepthBuffer.h"
#include "Graphics\GraphicsFeatures.h"


namespace Kodiak
{

struct ImageDesc
{
	Format format =				Format::Unknown;
	ResourceType type =			ResourceType::Unknown;
	GpuImageUsage usage =		GpuImageUsage::Unknown;
	MemoryAccess access =		MemoryAccess::Unknown;
	uint32_t width =			1;
	uint32_t height =			1;
	uint32_t depthOrArraySize = 1;
	uint32_t numMips =			1;
	uint32_t numSamples =		1;

	uint32_t GetDepth() const { return type == ResourceType::Texture3D ? depthOrArraySize : 1; }
	uint32_t GetArraySize() const { return IsTextureArray(type) ? depthOrArraySize : 1; }
};


struct BufferDesc
{
	Format format =					Format::Unknown;
	ResourceType type =				ResourceType::Unknown;
	MemoryAccess access =			MemoryAccess::Unknown;
	uint32_t numElements =			0;
	uint32_t elementSizeInBytes =	0;
	uint32_t bufferSizeInBytes =	0;
};


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
	KODIAK_NODISCARD VkResult CreateSemaphore(VkSemaphoreType semaphoreType, uint64_t initialValue, UVkSemaphore** ppSemaphore) const;
	KODIAK_NODISCARD VkResult CreateAllocator(UVmaAllocator** ppAllocator) const;
	KODIAK_NODISCARD VkResult CreateQueryPool(QueryHeapType type, uint32_t queryCount, UVkQueryPool** ppPool) const;
	KODIAK_NODISCARD VkResult CreateCommandPool(uint32_t queueFamilyIndex, UVkCommandPool** ppPool) const;
	KODIAK_NODISCARD VkResult CreateImageView(UVkImage* uimage, ResourceType type, GpuImageUsage imageUsage, Format format, ImageAspect aspect, uint32_t baseMipLevel, uint32_t mipCount, uint32_t baseArraySlice, uint32_t arraySize, UVkImageView** ppImageView) const;
	KODIAK_NODISCARD VkResult CreateBufferView(UVkBuffer* ubuffer, ResourceType type, Format format, uint32_t offsetInBytes, uint32_t sizeInBytes, UVkBufferView** ppBufferView) const;
	KODIAK_NODISCARD VkResult CreateRenderPass(const std::vector<ColorBufferPtr>& colorBuffers, DepthBufferPtr depthBuffer, UVkRenderPass** ppRenderPass) const;
	KODIAK_NODISCARD VkResult CreateFramebuffer(const std::vector<ColorBufferPtr>& colorBuffers, DepthBufferPtr depthBuffer, VkRenderPass renderPass, bool* bImageless, UVkFramebuffer** ppFramebuffer) const;
	KODIAK_NODISCARD VkResult CreateBuffer(const std::string& name, const BufferDesc& desc, UVkBuffer** ppBuffer) const;
	KODIAK_NODISCARD VkResult CreateImage(const std::string& name, const ImageDesc& desc, UVkImage** ppImage) const;
	KODIAK_NODISCARD VkResult CreatePipelineCache(UVkPipelineCache** ppPipelineCache) const;
	KODIAK_NODISCARD VkResult CreateGraphicsPipeline(const VkGraphicsPipelineCreateInfo& createInfo, UVkPipeline** ppPipeline) const;
	KODIAK_NODISCARD VkResult CreateComputePipeline(const VkComputePipelineCreateInfo& createInfo, UVkPipeline** ppPipeline) const;

	Format GetColorFormat() const { return m_colorFormat; }
	Format GetDepthFormat() const { return m_depthFormat; }

	ColorBufferPtr GetBackBuffer(uint32_t index) const;
	uint32_t GetCurrentBuffer() const { return m_currentBuffer; }

	uint32_t GetFrameNumber() const { return m_frameNumber; }

	const std::string& GetDeviceName() const { return m_deviceName; }

	VmaAllocator GetAllocator() const { return m_allocator->Get(); }

	void ReleaseResource(UVkImage* image);
	void ReleaseResource(UVkBuffer* buffer);

	uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound) const;
	VkFormatProperties GetFormatProperties(Format format) const;
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
		Microsoft::WRL::ComPtr<UVkImage> image;
		Microsoft::WRL::ComPtr<UVkBuffer> buffer;
	};
	std::list<DeferredReleaseResource> m_deferredResources;

	// Platform-specific implementation
	struct PlatformData;
	PlatformData* m_platformData{ nullptr };

	// Vulkan members (ref-counted)
	Microsoft::WRL::ComPtr<UVkInstance> m_instance;
	Microsoft::WRL::ComPtr<UVkPhysicalDevice> m_physicalDevice;
	Microsoft::WRL::ComPtr<UVkDevice> m_device;
	Microsoft::WRL::ComPtr<UVkSemaphore> m_imageAcquireSemaphore;
	Microsoft::WRL::ComPtr<UVkSemaphore> m_presentSemaphore;
	Microsoft::WRL::ComPtr<UVmaAllocator> m_allocator;
	Microsoft::WRL::ComPtr<UVkSurface> m_surface;
	Microsoft::WRL::ComPtr<UVkSwapchain> m_swapchain;
	Microsoft::WRL::ComPtr<UVkDebugUtilsMessenger> m_debugUtilsMessenger;
	Microsoft::WRL::ComPtr<UVkPipelineCache> m_pipelineCache;

	// Swapchain images
	std::vector<Microsoft::WRL::ComPtr<UVkImage>> m_swapchainImages;

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


VkDevice GetDevice();
VmaAllocator GetAllocator();


uint32_t GetMemoryTypeIndex(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32* memTypeFound = nullptr);
VkFormatProperties GetFormatProperties(Format format);

} // namespace Kodiak