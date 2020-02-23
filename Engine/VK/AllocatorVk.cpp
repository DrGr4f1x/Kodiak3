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

#include "AllocatorVk.h"

#include "InstanceVk.h"
#include "LogicalDeviceVk.h"
#include "PhysicalDeviceVk.h"

#define VMA_IMPLEMENTATION
#include "Extern\VulkanMemoryAllocator\vk_mem_alloc.h"

using namespace Kodiak;
using namespace std;


Allocator::Allocator(
	const shared_ptr<Instance>& instance, 
	const shared_ptr<PhysicalDevice>& physicalDevice, 
	const shared_ptr<LogicalDevice>& logicalDevice)
	: Reference(instance, physicalDevice, logicalDevice)
{
	Initialize();
}


Allocator::~Allocator()
{
	vmaDestroyAllocator(m_allocator);
	m_allocator = VK_NULL_HANDLE;
}


shared_ptr<Allocator> Allocator::Create(
	const shared_ptr<Instance>& instance,
	const shared_ptr<PhysicalDevice>& physicalDevice,
	const shared_ptr<LogicalDevice>& logicalDevice)
{
	shared_ptr<Allocator> allocator(new Allocator(instance, physicalDevice, logicalDevice));
	return allocator;
}


void Allocator::Initialize()
{
	VmaAllocatorCreateInfo createInfo = {};
	createInfo.physicalDevice = *Get<PhysicalDevice>();
	createInfo.device = *Get<LogicalDevice>();
	//createInfo.instance = *Get<Instance>();
	//createInfo.vulkanApiVersion = Get<Instance>()->GetVersion();

	ThrowIfFailed(vmaCreateAllocator(&createInfo, &m_allocator));
}