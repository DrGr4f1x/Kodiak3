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

#include "NonCopyable.h"


namespace Kodiak
{

template <typename... Ts>
class Reference
{};


template <class U, typename T, typename... Ts>
class ReferenceInternal
{
public:
	static const std::shared_ptr<U>& Get(const Reference<T, Ts...>& ref)
	{
		return ref.m_tail.template Get<U>();
	}

	static void Set(Reference<T, Ts...>& ref, const std::shared_ptr<U>& ptr)
	{
		ref.m_tail.template Set<U>(ptr);
	}
};


template <typename T, typename... Ts>
class ReferenceInternal<T, T, Ts...>
{
public:
	static const std::shared_ptr<T>& Get(const Reference<T, Ts...>& ref)
	{
		return ref.m_head;
	}

	static void Set(Reference<T, Ts...>& ref, const std::shared_ptr<T>& ptr)
	{
		ref.m_head = ptr;
	}
};


template <typename T, typename... Ts>
class Reference<T, Ts...>
{
	template <class U, typename X, typename... Xs>
	friend class ReferenceInternal;

public:
	Reference(const std::shared_ptr<T>& t, std::shared_ptr<Ts>... ts)
		: m_head(t)
		, m_tail(ts...)
	{}

	template <class U>
	const std::shared_ptr<U>& Get() const
	{
		return ReferenceInternal<U, T, Ts...>::Get(*this);
	}

protected:
	template <class U>
	void Set(const std::shared_ptr<U>& ptr)
	{
		ReferenceInternal<U, T, Ts...>::Set(*this, ptr);
	}

private:
	std::shared_ptr<T>	m_head;
	Reference<Ts...>	m_tail;
};


class InstanceRef : public std::enable_shared_from_this<InstanceRef>, public NonCopyable
{
public:
	virtual ~InstanceRef();

	static std::shared_ptr<InstanceRef> Create(VkInstance instance);

	operator VkInstance() { return m_instance; }

private:
	InstanceRef(VkInstance instance)
		: m_instance(instance)
	{}

private:
	VkInstance m_instance{ VK_NULL_HANDLE };
};


class PhysicalDeviceRef : public Reference<InstanceRef>, public NonCopyable
{
public:
	virtual ~PhysicalDeviceRef();

	static std::shared_ptr<PhysicalDeviceRef> Create(const std::shared_ptr<InstanceRef>& instance, VkPhysicalDevice physicalDevice);

	operator VkPhysicalDevice() { return m_physicalDevice; }

private:
	PhysicalDeviceRef(const std::shared_ptr<InstanceRef>& instance, VkPhysicalDevice physicalDevice)
		: Reference(instance)
		, m_physicalDevice(physicalDevice)
	{}

private:
	VkPhysicalDevice m_physicalDevice{ VK_NULL_HANDLE };
};


class SurfaceRef : public Reference<InstanceRef>, public NonCopyable
{
public:
	virtual ~SurfaceRef();

	static std::shared_ptr<SurfaceRef> Create(const std::shared_ptr<InstanceRef>& instance, VkSurfaceKHR surface);

	operator VkSurfaceKHR() { return m_surface; }

private:
	SurfaceRef(const std::shared_ptr<InstanceRef>& instance, VkSurfaceKHR surface)
		: Reference(instance)
		, m_surface(surface)
	{}

private:
	VkSurfaceKHR m_surface{ VK_NULL_HANDLE };
};


class DeviceRef : public Reference<PhysicalDeviceRef>, public NonCopyable
{
public:
	virtual ~DeviceRef();

	static std::shared_ptr<DeviceRef> Create(const std::shared_ptr<PhysicalDeviceRef>& physicalDevice, VkDevice device);

	operator VkDevice() { return m_device; }

private:
	DeviceRef(const std::shared_ptr<PhysicalDeviceRef>& physicalDevice, VkDevice device)
		: Reference(physicalDevice)
		, m_device(device)
	{}

private:
	VkDevice m_device{ VK_NULL_HANDLE };
};


class AllocatorRef : public Reference<InstanceRef, PhysicalDeviceRef, DeviceRef>, public NonCopyable
{
public:
	virtual ~AllocatorRef();

	static std::shared_ptr<AllocatorRef> Create(
		const std::shared_ptr<InstanceRef>& instance,
		const std::shared_ptr<PhysicalDeviceRef>& physicalDevice,
		const std::shared_ptr<DeviceRef>& device,
		VmaAllocator allocator);

	operator VmaAllocator() { return m_allocator; }

private:
	AllocatorRef(
		const std::shared_ptr<InstanceRef>& instance,
		const std::shared_ptr<PhysicalDeviceRef>& physicalDevice,
		const std::shared_ptr<DeviceRef>& device,
		VmaAllocator allocator)
		: Reference(instance, physicalDevice, device)
		, m_allocator{allocator}
	{}

private:
	VmaAllocator m_allocator{ VK_NULL_HANDLE };
};


class FenceRef : public Reference<DeviceRef>, public NonCopyable
{
public:
	virtual ~FenceRef();

	static std::shared_ptr<FenceRef> Create(const std::shared_ptr<DeviceRef>& device, VkFence fence);

	operator VkFence() { return m_fence; }

private:
	FenceRef(const std::shared_ptr<DeviceRef>& device, VkFence fence)
		: Reference(device)
		, m_fence(fence)
	{}

private:
	VkFence m_fence{ VK_NULL_HANDLE };
};


class SemaphoreRef : public Reference<DeviceRef>, public NonCopyable
{
public:
	virtual ~SemaphoreRef();

	static std::shared_ptr<SemaphoreRef> Create(const std::shared_ptr<DeviceRef>& device, VkSemaphore semaphore);

	operator VkSemaphore() { return m_semaphore; }

private:
	SemaphoreRef(const std::shared_ptr<DeviceRef>& device, VkSemaphore semaphore)
		: Reference(device)
		, m_semaphore(semaphore)
	{}

private:
	VkSemaphore m_semaphore{ VK_NULL_HANDLE };
};


class DebugReportCallbackRef : public Reference<InstanceRef>, public NonCopyable
{
public:
	virtual ~DebugReportCallbackRef();

	static std::shared_ptr<DebugReportCallbackRef> Create(const std::shared_ptr<InstanceRef>& instance, VkDebugReportCallbackEXT callback);

	operator VkDebugReportCallbackEXT() { return m_callback; }

private:
	DebugReportCallbackRef(const std::shared_ptr<InstanceRef>& instance, VkDebugReportCallbackEXT callback)
		: Reference(instance)
		, m_callback(callback)
	{}

private:
	VkDebugReportCallbackEXT m_callback{ VK_NULL_HANDLE };
};


class ImageRef : public Reference<DeviceRef, AllocatorRef>, public NonCopyable
{
public:
	virtual ~ImageRef();

	static std::shared_ptr<ImageRef> Create(const std::shared_ptr<DeviceRef>& device, VkImage image);
	static std::shared_ptr<ImageRef> Create(
		const std::shared_ptr<DeviceRef>& device,
		const std::shared_ptr<AllocatorRef>& allocator,
		VkImage image,
		VmaAllocation allocation);

	operator VkImage() { return m_image; }

private:
	ImageRef(const std::shared_ptr<DeviceRef>& device, VkImage image)
		: Reference(device, nullptr)
		, m_image(image)
	{}

	ImageRef(
		const std::shared_ptr<DeviceRef>& device,
		const std::shared_ptr<AllocatorRef>& allocator,
		VkImage image,
		VmaAllocation allocation)
		: Reference(device, allocator)
		, m_image(image)
		, m_allocation(allocation)
	{}

private:
	VkImage m_image{ VK_NULL_HANDLE };
	VmaAllocation m_allocation{ VK_NULL_HANDLE };
};


class SwapchainRef : public Reference<DeviceRef>, public NonCopyable
{
public:
	virtual ~SwapchainRef();

	static std::shared_ptr<SwapchainRef> Create(const std::shared_ptr<DeviceRef>& device, VkSwapchainKHR swapchain);

	operator VkSwapchainKHR() { return m_swapchain; }

private:
	SwapchainRef(const std::shared_ptr<DeviceRef>& device, VkSwapchainKHR swapchain)
		: Reference(device)
		, m_swapchain(swapchain)
	{}

private:
	VkSwapchainKHR m_swapchain{ VK_NULL_HANDLE };
};

} // namespace Kodiak