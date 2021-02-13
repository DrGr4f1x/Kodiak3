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


void GpuBuffer::Create(const string& name, size_t numElements, size_t elementSize, bool allowCpuWrites, const void* initialData)
{
	if (m_buffer)
	{
		g_graphicsDevice->ReleaseResource(m_buffer.Get());
		m_buffer = nullptr;
	}

	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	BufferDesc desc = {};
	desc.type = m_type;
	desc.access = MemoryAccess::GpuRead | MemoryAccess::GpuWrite;
	desc.access |= allowCpuWrites ? MemoryAccess::CpuWrite : MemoryAccess::Unknown;
	desc.numElements = uint32_t(m_elementCount);
	desc.elementSizeInBytes = uint32_t(m_elementSize);
	desc.bufferSizeInBytes = uint32_t(m_bufferSize);

	ThrowIfFailed(g_graphicsDevice->CreateBuffer(name, desc, &m_buffer));

	// Upload to GPU
	if (initialData && !allowCpuWrites)
	{
		CommandContext::InitializeBuffer(*this, initialData, m_bufferSize);
	}

	CreateDerivedViews();
}


void IndexBuffer::Update(size_t sizeInBytes, const void* data)
{
	Update(sizeInBytes, 0, data);
}


void IndexBuffer::Update(size_t sizeInBytes, size_t offset, const void* data)
{
	assert((sizeInBytes + offset) <= m_bufferSize);
	// TODO Handle non-zero offsets
	assert(offset == 0);

	// Map uniform buffer and update it
	uint8_t* pData = nullptr;
	ThrowIfFailed(vmaMapMemory(m_buffer->GetAllocator(), m_buffer->GetAllocation(), (void **)&pData));
	memcpy(pData, data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	vmaUnmapMemory(m_buffer->GetAllocator(), m_buffer->GetAllocation());
}


void VertexBuffer::Update(size_t sizeInBytes, const void* data)
{
	Update(sizeInBytes, 0, data);
}


void VertexBuffer::Update(size_t sizeInBytes, size_t offset, const void* data)
{
	assert((sizeInBytes + offset) <= m_bufferSize);
	// TODO Handle non-zero offsets
	assert(offset == 0);

	// Map uniform buffer and update it
	uint8_t* pData = nullptr;
	ThrowIfFailed(vmaMapMemory(m_buffer->GetAllocator(), m_buffer->GetAllocation(), (void**)&pData));
	memcpy(pData, data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	vmaUnmapMemory(m_buffer->GetAllocator(), m_buffer->GetAllocation());
}


void ConstantBuffer::Update(size_t sizeInBytes, const void* data)
{
	Update(sizeInBytes, 0, data);
}


void ConstantBuffer::Update(size_t sizeInBytes, size_t offset, const void* data)
{
	assert((sizeInBytes + offset) <= m_bufferSize);

	// Map uniform buffer and update it
	uint8_t* pData = nullptr;
	ThrowIfFailed(vmaMapMemory(m_buffer->GetAllocator(), m_buffer->GetAllocation(), (void**)&pData));
	memcpy(pData + offset, data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	vmaUnmapMemory(m_buffer->GetAllocator(), m_buffer->GetAllocation());
}


void ReadbackBuffer::Create(const string& name, uint32_t numElements, uint32_t elementSize)
{
	if (m_buffer)
	{
		g_graphicsDevice->ReleaseResource(m_buffer.Get());
		m_buffer = nullptr;
	}

	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	BufferDesc desc = {};
	desc.type = m_type;
	desc.access = MemoryAccess::GpuRead | MemoryAccess::GpuWrite | MemoryAccess::CpuRead;
	desc.numElements = uint32_t(m_elementCount);
	desc.elementSizeInBytes = uint32_t(m_elementSize);
	desc.bufferSizeInBytes = uint32_t(m_bufferSize);

	ThrowIfFailed(g_graphicsDevice->CreateBuffer(name, desc, &m_buffer));

	CreateDerivedViews();
}


void* ReadbackBuffer::Map()
{
	void* mem = nullptr;
	ThrowIfFailed(vmaMapMemory(m_buffer->GetAllocator(), m_buffer->GetAllocation(), &mem));
	return mem;
}


void ReadbackBuffer::Unmap()
{
	vmaUnmapMemory(m_buffer->GetAllocator(), m_buffer->GetAllocation());
}


GpuBuffer::~GpuBuffer()
{
	g_graphicsDevice->ReleaseResource(m_buffer.Get());
}


void IndexBuffer::CreateDerivedViews()
{
	m_indexSize16 = (m_elementSize == 2);
}


void ConstantBuffer::CreateDerivedViews()
{
	m_bufferInfo = { m_buffer->Get(), 0, VK_WHOLE_SIZE };
}


void ByteAddressBuffer::CreateDerivedViews()
{
	m_bufferInfo = { m_buffer->Get(), 0, VK_WHOLE_SIZE };
}


void StructuredBuffer::CreateDerivedViews()
{
	m_bufferInfo = { m_buffer->Get(), 0, VK_WHOLE_SIZE };

	m_counterBuffer.Create("Counter Buffer", 1, 4, false);
}


const VkDescriptorBufferInfo* StructuredBuffer::GetCounterSRVBufferInfoPtr(CommandContext& context)
{
	context.TransitionResource(m_counterBuffer, ResourceState::GenericRead);
	return m_counterBuffer.GetBufferInfoPtr();
}


const VkDescriptorBufferInfo* StructuredBuffer::GetCounterUAVBufferInfoPtr(CommandContext& context)
{
	context.TransitionResource(m_counterBuffer, ResourceState::UnorderedAccess);
	return m_counterBuffer.GetBufferInfoPtr();
}


void TypedBuffer::CreateDerivedViews()
{
	m_bufferInfo = { m_buffer->Get(), 0, VK_WHOLE_SIZE };
}