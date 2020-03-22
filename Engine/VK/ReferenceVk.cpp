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

#include "ReferenceVk.h"


using namespace Kodiak;
using namespace std;


InstanceRef::~InstanceRef()
{
	vkDestroyInstance(m_instance, nullptr);
	m_instance = VK_NULL_HANDLE;
}


shared_ptr<InstanceRef> InstanceRef::Create(VkInstance instance)
{
	shared_ptr<InstanceRef> ptr(new InstanceRef(instance));
	return ptr;
}


PhysicalDeviceRef::~PhysicalDeviceRef()
{
	m_physicalDevice = VK_NULL_HANDLE;
}


shared_ptr<PhysicalDeviceRef> PhysicalDeviceRef::Create(const shared_ptr<InstanceRef>& instance, VkPhysicalDevice physicalDevice)
{
	shared_ptr<PhysicalDeviceRef> ptr(new PhysicalDeviceRef(instance, physicalDevice));
	return ptr;
}


SurfaceRef::~SurfaceRef()
{
	vkDestroySurfaceKHR(*Get<InstanceRef>(), m_surface, nullptr);
	m_surface = VK_NULL_HANDLE;
}


shared_ptr<SurfaceRef> SurfaceRef::Create(const shared_ptr<InstanceRef>& instance, VkSurfaceKHR surface)
{
	shared_ptr<SurfaceRef> ptr(new SurfaceRef(instance, surface));
	return ptr;
}


DeviceRef::~DeviceRef()
{
	vkDestroyDevice(m_device, nullptr);
	m_device = VK_NULL_HANDLE;
}


shared_ptr<DeviceRef> DeviceRef::Create(const shared_ptr<PhysicalDeviceRef>& physicalDevice, VkDevice device)
{
	shared_ptr<DeviceRef> ptr(new DeviceRef(physicalDevice, device));
	return ptr;
}


AllocatorRef::~AllocatorRef()
{
	vmaDestroyAllocator(m_allocator);
	m_allocator = VK_NULL_HANDLE;
}


shared_ptr<AllocatorRef> AllocatorRef::Create(
	const shared_ptr<InstanceRef>& instance,
	const shared_ptr<PhysicalDeviceRef>& physicalDevice,
	const shared_ptr<DeviceRef>& device,
	VmaAllocator allocator)
{
	shared_ptr<AllocatorRef> ptr(new AllocatorRef(instance, physicalDevice, device, allocator));
	return ptr;
}


FenceRef::~FenceRef()
{
	vkDestroyFence(*Get<DeviceRef>(), m_fence, nullptr);
	m_fence = VK_NULL_HANDLE;
}


shared_ptr<FenceRef> FenceRef::Create(const shared_ptr<DeviceRef>& device, VkFence fence)
{
	shared_ptr<FenceRef> ptr(new FenceRef(device, fence));
	return ptr;
}


SemaphoreRef::~SemaphoreRef()
{
	vkDestroySemaphore(*Get<DeviceRef>(), m_semaphore, nullptr);
	m_semaphore = VK_NULL_HANDLE;
}


shared_ptr<SemaphoreRef> SemaphoreRef::Create(const shared_ptr<DeviceRef>& device, VkSemaphore semaphore)
{
	shared_ptr<SemaphoreRef> ptr(new SemaphoreRef(device, semaphore));
	return ptr;
}


DebugReportCallbackRef::~DebugReportCallbackRef()
{
#if ENABLE_VULKAN_VALIDATION
	extern PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallback;
	vkDestroyDebugReportCallback(*Get<InstanceRef>(), m_callback, nullptr);
#endif
	m_callback = VK_NULL_HANDLE;
}


shared_ptr<DebugReportCallbackRef> DebugReportCallbackRef::Create(const shared_ptr<InstanceRef>& instance, VkDebugReportCallbackEXT callback)
{
	shared_ptr<DebugReportCallbackRef> ptr(new DebugReportCallbackRef(instance, callback));
	return ptr;
}


ImageRef::~ImageRef()
{
	if (m_allocation != VK_NULL_HANDLE)
	{
		vmaDestroyImage(*Get<AllocatorRef>(), m_image, m_allocation);
		m_allocation = VK_NULL_HANDLE;
	}
	m_image = VK_NULL_HANDLE;
}


shared_ptr<ImageRef> ImageRef::Create(const shared_ptr<DeviceRef>& device, VkImage image)
{
	shared_ptr<ImageRef> ptr(new ImageRef(device, image));
	return ptr;
}


shared_ptr<ImageRef> ImageRef::Create(
	const shared_ptr<DeviceRef>& device,
	const shared_ptr<AllocatorRef>& allocator,
	VkImage image,
	VmaAllocation allocation)
{
	shared_ptr<ImageRef> ptr(new ImageRef(device, allocator, image, allocation));
	return ptr;
}


SwapchainRef::~SwapchainRef()
{
	vkDestroySwapchainKHR(*Get<DeviceRef>(), m_swapchain, nullptr);
	m_swapchain = VK_NULL_HANDLE;
}


shared_ptr<SwapchainRef> SwapchainRef::Create(const shared_ptr<DeviceRef>& device, VkSwapchainKHR swapchain)
{
	shared_ptr<SwapchainRef> ptr(new SwapchainRef(device, swapchain));
	return ptr;
}