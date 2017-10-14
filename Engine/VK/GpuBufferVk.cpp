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

#include "GpuBufferVk.h"

#include "CommandContextVk.h"
#include "GraphicsDeviceVk.h"


using namespace Kodiak;
using namespace std;


void GpuBuffer::Destroy()
{
	vkDestroyBuffer(GetDevice(), m_buffer, nullptr);
	m_buffer = VK_NULL_HANDLE;
	
	GpuResource::Destroy();
}


void GpuBuffer::CreateBuffer(const string& name, uint32_t numElements, uint32_t elementSize, VkBufferUsageFlagBits flags, bool bHostMappable, const void* initialData)
{
	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	VkBufferCreateInfo vertexbufferInfo = {};
	vertexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexbufferInfo.size = m_bufferSize;
	vertexbufferInfo.usage = flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	ThrowIfFailed(vkCreateBuffer(GetDevice(), &vertexbufferInfo, nullptr, &m_buffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(GetDevice(), m_buffer, &memReqs);

	VkMemoryPropertyFlags memFlags = bHostMappable ?
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT :
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryType(memReqs.memoryTypeBits, memFlags);

	ThrowIfFailed(vkAllocateMemory(GetDevice(), &memAlloc, nullptr, &m_deviceMemory));

	if (initialData && bHostMappable)
	{
		void* data = nullptr;
		ThrowIfFailed(vkMapMemory(GetDevice(), m_deviceMemory, 0, m_bufferSize, 0, &data));
		memcpy(data, initialData, m_bufferSize);
		vkUnmapMemory(GetDevice(), m_deviceMemory);
	}

	ThrowIfFailed(vkBindBufferMemory(GetDevice(), m_buffer, m_deviceMemory, 0));

	SetDebugName(m_buffer, name);
	SetDebugName(m_deviceMemory, name + " memory");

	// Upload to GPU
	if (initialData && !bHostMappable)
	{
		CommandContext::InitializeBuffer(*this, initialData, m_bufferSize);
	}

	m_descriptorInfo.buffer = m_buffer;
	m_descriptorInfo.offset = 0;
	m_descriptorInfo.range = m_bufferSize;
}