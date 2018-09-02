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
#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


void GpuBuffer::Destroy()
{
	// TODO
	m_resource = nullptr;
}


void GpuBuffer::CreateBuffer(const string& name, size_t numElements, size_t elementSize, VkBufferUsageFlagBits flags, bool bHostMappable, const void* initialData)
{
	VkDevice device = GetDevice();

	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	VkBufferCreateInfo vertexbufferInfo = {};
	vertexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexbufferInfo.size = m_bufferSize;
	vertexbufferInfo.usage = flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkBuffer buffer{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateBuffer(device, &vertexbufferInfo, nullptr, &buffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, buffer, &memReqs);

	VkMemoryPropertyFlags memFlags = bHostMappable ?
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT :
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, memFlags);

	VkDeviceMemory mem{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(device, &memAlloc, nullptr, &mem));
	m_resource = ResourceHandle::Create(buffer, mem);

	if (initialData && bHostMappable)
	{
		void* data = nullptr;
		ThrowIfFailed(vkMapMemory(device, m_resource, 0, m_bufferSize, 0, &data));
		memcpy(data, initialData, m_bufferSize);
		vkUnmapMemory(device, m_resource);
	}

	ThrowIfFailed(vkBindBufferMemory(device, buffer, m_resource, 0));

	// TODO
	//SetDebugName(m_buffer, name);
	//SetDebugName(*m_resource, name + " memory");

	// Upload to GPU
	if (initialData && !bHostMappable)
	{
		CommandContext::InitializeBuffer(*this, initialData, m_bufferSize);
	}

	m_descriptorInfo.buffer = buffer;
	m_descriptorInfo.offset = 0;
	m_descriptorInfo.range = m_bufferSize;
}


void IndexBuffer::Create(const string& name, size_t numElements, size_t elementSize, const void* initialData)
{
	assert(elementSize == 2 || elementSize == 4);
	const bool bHostMappable = false;
	CreateBuffer(name, numElements, elementSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, bHostMappable, initialData);

	m_indexType = elementSize == 2 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
}


void VertexBuffer::Create(const string& name, size_t numElements, size_t elementSize, const void* initialData)
{
	const bool bHostMappable = false;
	CreateBuffer(name, numElements, elementSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, bHostMappable, initialData);
}


void ConstantBuffer::Create(const string& name, size_t numElements, size_t elementSize, const void* initialData)
{
	const bool bHostMappable = true;
	CreateBuffer(name, 1, elementSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, bHostMappable, initialData);
}


void ConstantBuffer::Update(size_t sizeInBytes, const void* data)
{
	assert(sizeInBytes <= m_bufferSize);

	VkDevice device = GetDevice();

	// Map uniform buffer and update it
	uint8_t* pData = nullptr;
	ThrowIfFailed(vkMapMemory(device, m_resource, 0, sizeInBytes, 0, (void **)&pData));
	memcpy(pData, data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	vkUnmapMemory(device, m_resource);
}


void ConstantBuffer::Update(size_t sizeInBytes, size_t offset, const void* data)
{
	assert((sizeInBytes + offset) <= m_bufferSize);

	VkDevice device = GetDevice();

	// Map uniform buffer and update it
	uint8_t* pData = nullptr;
	ThrowIfFailed(vkMapMemory(device, m_resource, offset, sizeInBytes, 0, (void **)&pData));
	memcpy(pData, data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	vkUnmapMemory(device, m_resource);
}