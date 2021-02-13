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


BufferViewDesc GpuBuffer::GetDesc() const
{
	BufferViewDesc resDesc = {};
	resDesc.format = Format::Unknown;
	resDesc.bufferSize = (uint32_t)m_bufferSize;
	resDesc.elementCount = (uint32_t)m_elementCount;
	resDesc.elementSize = (uint32_t)m_elementSize;
	return resDesc;
}


void IndexBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	//m_srv.Create(m_resource, m_type, resDesc);
	//m_uav.Create(m_resource, m_type, resDesc);

	m_indexSize16 = (m_elementSize == 2);
}


void VertexBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	//m_srv.Create(m_resource, m_type, resDesc);
	//m_uav.Create(m_resource, m_type, resDesc);
}


void ConstantBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_cbv.Create(m_buffer.Get(), resDesc);
}


void ByteAddressBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_srv.Create(m_buffer.Get(), m_type, resDesc);
	m_uav.Create(m_buffer.Get(), m_type, resDesc);
}


void StructuredBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_srv.Create(m_buffer.Get(), ResourceType::StructuredBuffer, resDesc);
	m_uav.Create(m_buffer.Get(), ResourceType::StructuredBuffer, resDesc);

	m_counterBuffer.Create("Counter Buffer", 1, 4, false);
}


const ShaderResourceView& StructuredBuffer::GetCounterSRV(CommandContext& context)
{
	context.TransitionResource(m_counterBuffer, ResourceState::GenericRead);
	return m_counterBuffer.GetSRV();
}


const UnorderedAccessView& StructuredBuffer::GetCounterUAV(CommandContext& context)
{
	context.TransitionResource(m_counterBuffer, ResourceState::UnorderedAccess);
	return m_counterBuffer.GetUAV();
}


void TypedBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();
	resDesc.format = m_dataFormat;

	m_srv.Create(m_buffer.Get(), m_type, resDesc);
	m_uav.Create(m_buffer.Get(), m_type, resDesc);
}