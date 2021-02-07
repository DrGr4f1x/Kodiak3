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

// GUIDs for IUnknown::QueryInterface
// {2CB7A094-EC8F-4160-84AA-F18746FEF0BD}
DEFINE_GUID(IID_UVkInstance, 0x2cb7a094, 0xec8f, 0x4160, 0x84, 0xaa, 0xf1, 0x87, 0x46, 0xfe, 0xf0, 0xbd);
// {DB19A237-4CDE-4609-9C89-19E46CBA854B}
DEFINE_GUID(IID_UVkPhysicalDevice, 0xdb19a237, 0x4cde, 0x4609, 0x9c, 0x89, 0x19, 0xe4, 0x6c, 0xba, 0x85, 0x4b);
// {9891D0BF-51A4-4A83-A6D8-44F6E3568243}
DEFINE_GUID(IID_UVkSurface,	0x9891d0bf, 0x51a4, 0x4a83, 0xa6, 0xd8, 0x44, 0xf6, 0xe3, 0x56, 0x82, 0x43);
// {1BE6F63B-6AC2-4C35-A0F3-E22D7957E146}
DEFINE_GUID(IID_UVkDevice, 0x1be6f63b, 0x6ac2, 0x4c35, 0xa0, 0xf3, 0xe2, 0x2d, 0x79, 0x57, 0xe1, 0x46);
// {A60BCD86-CE2B-4C5F-A73D-B842A271CEF9}
DEFINE_GUID(IID_UVmaAllocator, 0xa60bcd86, 0xce2b, 0x4c5f, 0xa7, 0x3d, 0xb8, 0x42, 0xa2, 0x71, 0xce, 0xf9);
// {E1EAA295-40E9-4E41-90DB-E1B1B2E6848E}
DEFINE_GUID(IID_UVkFence, 0xe1eaa295, 0x40e9, 0x4e41, 0x90, 0xdb, 0xe1, 0xb1, 0xb2, 0xe6, 0x84, 0x8e);
// {10F03D1D-FC84-4462-8759-FAFE672E052A}
DEFINE_GUID(IID_UVkSemaphore, 0x10f03d1d, 0xfc84, 0x4462, 0x87, 0x59, 0xfa, 0xfe, 0x67, 0x2e, 0x5, 0x2a);
// {100F605A-002B-48D0-809C-2BFB86192CAD}
DEFINE_GUID(IID_UVkDebugUtilsMessenger, 0x100f605a, 0x2b, 0x48d0, 0x80, 0x9c, 0x2b, 0xfb, 0x86, 0x19, 0x2c, 0xad);
// {D12387CD-09E8-42EF-AA25-E9FC5E29292C}
DEFINE_GUID(IID_UVkImage, 0xd12387cd, 0x9e8, 0x42ef, 0xaa, 0x25, 0xe9, 0xfc, 0x5e, 0x29, 0x29, 0x2c);
// {2062E1BA-E3B9-4E43-A9BF-1F72AE5B0A6F}
DEFINE_GUID(IID_UVkSwapchain, 0x2062e1ba, 0xe3b9, 0x4e43, 0xa9, 0xbf, 0x1f, 0x72, 0xae, 0x5b, 0xa, 0x6f);


// Macro to define the IUnknown interface
#define IMPLEMENT_IUNKNOWN(IID_TYPE) \
HRESULT QueryInterface(REFIID riid, LPVOID* ppvObj) final \
{ \
	if (!ppvObj) \
	{ \
		return E_INVALIDARG; \
	} \
	*ppvObj = nullptr; \
	if (riid == IID_IUnknown || riid == IID_TYPE) \
	{ \
		*ppvObj = (LPVOID)this; \
		AddRef(); \
		return NOERROR; \
	} \
	return E_NOINTERFACE; \
} \
\
ULONG AddRef() final \
{ \
	++m_cRef; \
	return m_cRef; \
} \
\
ULONG Release() final \
{ \
	ULONG ulRefCount = --m_cRef; \
	if (0 == m_cRef) \
	{ \
		delete this; \
	} \
	return ulRefCount; \
}\
\
private: \
	std::atomic_ulong m_cRef{ 0 };


//
// VkInstance
//
class UVkInstance : public IUnknown, public NonCopyable
{
public:
	explicit UVkInstance(VkInstance instance)
		: m_instance(instance)
	{}
	~UVkInstance()
	{
		if (m_instance)
		{
			vkDestroyInstance(m_instance, nullptr);
			m_instance = VK_NULL_HANDLE;
		}
	}

	VkInstance Get() const { return m_instance; }

	operator VkInstance() const
	{
		return m_instance;
	}

	IMPLEMENT_IUNKNOWN(IID_UVkInstance)

private:
	VkInstance m_instance{ VK_NULL_HANDLE };
};


//
// VkPhysicalDevice
//
class UVkPhysicalDevice : public IUnknown, public NonCopyable
{
public:
	UVkPhysicalDevice(UVkInstance* uinstance, VkPhysicalDevice physicalDevice)
		: m_instance(uinstance)
		, m_physicalDevice(physicalDevice)
	{}
	~UVkPhysicalDevice()
	{
		m_physicalDevice = VK_NULL_HANDLE;
	}

	VkPhysicalDevice Get() const { return m_physicalDevice; }

	operator VkPhysicalDevice() const
	{
		return m_physicalDevice;
	}

	IMPLEMENT_IUNKNOWN(IID_UVkPhysicalDevice)

private:
	Microsoft::WRL::ComPtr<UVkInstance> m_instance{ nullptr };
	VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };
};


//
// VkSurfaceKHR
//
class UVkSurface : public IUnknown, public NonCopyable
{
public:
	UVkSurface(UVkInstance* uinstance, VkSurfaceKHR surface)
		: m_instance(uinstance)
		, m_surfaceKHR(surface)
	{}
	~UVkSurface()
	{
		if (m_surfaceKHR)
		{
			vkDestroySurfaceKHR(m_instance->Get(), m_surfaceKHR, nullptr);
			m_surfaceKHR = VK_NULL_HANDLE;
		}
	}

	VkSurfaceKHR Get() const { return m_surfaceKHR; }

	operator VkSurfaceKHR() const
	{
		return m_surfaceKHR;
	}

	IMPLEMENT_IUNKNOWN(IID_UVkSurface)

private:
	Microsoft::WRL::ComPtr<UVkInstance> m_instance{ nullptr };
	VkSurfaceKHR m_surfaceKHR{ VK_NULL_HANDLE };
};


//
// VkDevice
//
class UVkDevice : public IUnknown, public NonCopyable
{
public:
	UVkDevice(UVkPhysicalDevice* uphysicalDevice, VkDevice device)
		: m_physicalDevice(uphysicalDevice)
		, m_device(device)
	{}
	~UVkDevice()
	{
		if (m_device)
		{
			vkDestroyDevice(m_device, nullptr);
			m_device = VK_NULL_HANDLE;
		}
	}

	VkDevice Get() const { return m_device; }

	operator VkDevice() const
	{
		return m_device;
	}

	IMPLEMENT_IUNKNOWN(IID_UVkDevice)

private:
	Microsoft::WRL::ComPtr<UVkPhysicalDevice> m_physicalDevice{ nullptr };
	VkDevice m_device{ VK_NULL_HANDLE };
};


//
// VmaAllocator
//
class UVmaAllocator : public IUnknown, public NonCopyable
{
public:
	UVmaAllocator(UVkDevice* udevice, VmaAllocator allocator)
		: m_device(udevice)
		, m_allocator(allocator)
	{}
	~UVmaAllocator()
	{
		if (m_allocator)
		{
			vmaDestroyAllocator(m_allocator);
			m_allocator = VK_NULL_HANDLE;
		}
	}

	VmaAllocator Get() const { return m_allocator; }

	operator VmaAllocator() const
	{
		return m_allocator;
	}

	IMPLEMENT_IUNKNOWN(IID_UVmaAllocator)

private:
	Microsoft::WRL::ComPtr<UVkDevice> m_device{ nullptr };
	VmaAllocator m_allocator{ VK_NULL_HANDLE };
};


//
// VkFence
//
class UVkFence : public IUnknown, public NonCopyable
{
public:
	UVkFence(UVkDevice* udevice, VkFence fence)
		: m_device(udevice)
		, m_fence(fence)
	{}
	~UVkFence()
	{
		if (m_fence)
		{
			vkDestroyFence(m_device->Get(), m_fence, nullptr);
			m_fence = VK_NULL_HANDLE;
		}
	}

	VkFence Get() const { return m_fence; }

	operator VkFence() const
	{
		return m_fence;
	}

	IMPLEMENT_IUNKNOWN(IID_UVkFence)

private:
	Microsoft::WRL::ComPtr<UVkDevice> m_device{ nullptr };
	VkFence m_fence{ VK_NULL_HANDLE };
};


//
// VkSemaphore
//
class UVkSemaphore : public IUnknown, public NonCopyable
{
public:
	UVkSemaphore(UVkDevice* udevice, VkSemaphore semaphore)
		: m_device(udevice)
		, m_semaphore(semaphore)
	{}
	~UVkSemaphore()
	{
		if (m_semaphore)
		{
			vkDestroySemaphore(m_device->Get(), m_semaphore, nullptr);
			m_semaphore = VK_NULL_HANDLE;
		}
	}

	VkSemaphore Get() const { return m_semaphore; }

	operator VkSemaphore() const
	{
		return m_semaphore;
	}

	IMPLEMENT_IUNKNOWN(IID_UVkSemaphore)

private:
	Microsoft::WRL::ComPtr<UVkDevice> m_device{ nullptr };
	VkSemaphore m_semaphore{ VK_NULL_HANDLE };
};


//
// VkDebugUtilsMessengerEXT
//
class UVkDebugUtilsMessenger : public IUnknown, public NonCopyable
{
public:
	UVkDebugUtilsMessenger(UVkInstance* uinstance, VkDebugUtilsMessengerEXT messenger)
		: m_instance(uinstance)
		, m_messenger(messenger)
	{}
	~UVkDebugUtilsMessenger()
	{
		if (m_messenger)
		{
#if USE_VALIDATION_LAYER
			extern PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessenger;
			vkDestroyDebugUtilsMessenger(m_instance->Get(), m_messenger, nullptr);
#endif
			m_messenger = VK_NULL_HANDLE;
		}
	}

	VkDebugUtilsMessengerEXT Get() const { return m_messenger; }

	operator VkDebugUtilsMessengerEXT() const
	{
		return m_messenger;
	}

	IMPLEMENT_IUNKNOWN(IID_UVkDebugUtilsMessenger)

private:
	Microsoft::WRL::ComPtr<UVkInstance> m_instance{ nullptr };
	VkDebugUtilsMessengerEXT m_messenger{ VK_NULL_HANDLE };
};


//
// VkImage
//
class UVkImage : public IUnknown, public NonCopyable
{
public:
	UVkImage(UVkDevice* udevice, VkImage image)
		: m_device(udevice)
		, m_allocator(nullptr)
		, m_image(image)
		, m_allocation(VK_NULL_HANDLE)
	{}
	UVkImage(UVkDevice* udevice, UVmaAllocator* uallocator, VkImage image, VmaAllocation allocation)
		: m_device(udevice)
		, m_allocator(uallocator)
		, m_image(image)
		, m_allocation(allocation)
	{}
	~UVkImage()
	{
		if (m_allocation)
		{
			vmaDestroyImage(m_allocator->Get(), m_image, m_allocation);
			m_allocation = VK_NULL_HANDLE;
		}
		m_image = VK_NULL_HANDLE;
	}

	VkImage Get() const { return m_image; }
	operator VkImage() const { return m_image; }

	IMPLEMENT_IUNKNOWN(IID_UVkImage)

private:
	Microsoft::WRL::ComPtr<UVkDevice> m_device{ nullptr };
	Microsoft::WRL::ComPtr<UVmaAllocator> m_allocator{ nullptr };
	VkImage m_image{ VK_NULL_HANDLE };
	VmaAllocation m_allocation{ VK_NULL_HANDLE };
};


//
// VkSwapchainKHR
//
class UVkSwapchain : public IUnknown, public NonCopyable
{
public:
	UVkSwapchain(UVkDevice* udevice, VkSwapchainKHR swapchain)
		: m_device(udevice)
		, m_swapchain(swapchain)
	{}
	~UVkSwapchain()
	{
		if (m_swapchain)
		{
			vkDestroySwapchainKHR(m_device->Get(), m_swapchain, nullptr);
			m_swapchain = VK_NULL_HANDLE;
		}
	}

	VkSwapchainKHR Get() const { return m_swapchain; }
	operator VkSwapchainKHR() const { return m_swapchain; }

	IMPLEMENT_IUNKNOWN(IID_UVkSwapchain)

private:
	Microsoft::WRL::ComPtr<UVkDevice> m_device{ nullptr };
	VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
};

} // namespace Kodiak