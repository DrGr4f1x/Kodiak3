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

#include "GpuBuffer.h"

#include "CommandContextVk.h"
#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


void GpuBuffer::Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData)
{
	m_resource = nullptr;

	VkDevice device = GetDevice();

	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	VkBufferUsageFlags flags = 0;
	flags |= (m_type == ResourceType::IndexBuffer) ? VK_BUFFER_USAGE_INDEX_BUFFER_BIT : 0;
	flags |= (m_type == ResourceType::VertexBuffer) ? VK_BUFFER_USAGE_VERTEX_BUFFER_BIT : 0;
	flags |= (m_type == ResourceType::TypedBuffer) ? (VK_BUFFER_USAGE_STORAGE_TEXEL_BUFFER_BIT | VK_BUFFER_USAGE_UNIFORM_TEXEL_BUFFER_BIT) : 0;
	flags |= (m_type == ResourceType::GenericBuffer) ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : 0;
	flags |= (m_type == ResourceType::StructuredBuffer) ? VK_BUFFER_USAGE_STORAGE_BUFFER_BIT : 0;
	flags |= m_isConstantBuffer ? VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT : 0;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = m_bufferSize;
	bufferInfo.usage = flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkBuffer buffer{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, buffer, &memReqs);

	const bool bHostMappable = m_isConstantBuffer;
	VkMemoryPropertyFlags memFlags = bHostMappable ?
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT :
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;

	VkBufferCreateInfo vertexbufferInfo = {};
	vertexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexbufferInfo.size = m_bufferSize;
	vertexbufferInfo.usage = flags | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

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

	SetDebugName(buffer, name);
	SetDebugName(mem, name + " memory");

	// Upload to GPU
	if (initialData && !bHostMappable)
	{
		CommandContext::InitializeBuffer(*this, initialData, m_bufferSize);
	}

	CreateDerivedViews();
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


void ReadbackBuffer::Create(const string& name, uint32_t numElements, uint32_t elementSize)
{
	VkDevice device = GetDevice();

	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = m_bufferSize;
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkBuffer buffer{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateBuffer(device, &bufferInfo, nullptr, &buffer));

	VkMemoryRequirements memReqs;
	vkGetBufferMemoryRequirements(device, buffer, &memReqs);

	const bool bHostMappable = m_isConstantBuffer;
	VkMemoryPropertyFlags memFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

	VkBufferCreateInfo vertexbufferInfo = {};
	vertexbufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexbufferInfo.size = m_bufferSize;
	vertexbufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkMemoryAllocateInfo memAlloc = {};
	memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memAlloc.allocationSize = memReqs.size;
	memAlloc.memoryTypeIndex = GetMemoryTypeIndex(memReqs.memoryTypeBits, memFlags);

	VkDeviceMemory mem{ VK_NULL_HANDLE };
	ThrowIfFailed(vkAllocateMemory(device, &memAlloc, nullptr, &mem));
	m_resource = ResourceHandle::Create(buffer, mem);

	ThrowIfFailed(vkBindBufferMemory(device, buffer, m_resource, 0));

	SetDebugName(buffer, name);
	SetDebugName(mem, name + " memory");
}


void* ReadbackBuffer::Map()
{
	void* mem = nullptr;
	ThrowIfFailed(vkMapMemory(GetDevice(), m_resource, 0, m_bufferSize, 0, &mem));
	return mem;
}


void ReadbackBuffer::Unmap()
{
	vkUnmapMemory(GetDevice(), m_resource);
}