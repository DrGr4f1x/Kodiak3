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

#include "GpuBuffer12.h"

#include "CommandContext12.h"
#include "GraphicsDevice12.h"
#include "Util12.h"


using namespace Kodiak;
using namespace std;


static inline bool HasFlag(ResourceType type, ResourceType flag)
{
	return (type & flag) != 0;
}


void GpuBuffer::Create(const string& name, size_t numElements, size_t elementSize, bool allowCpuWrites, const void* initialData)
{
	if (m_resource)
	{
		g_graphicsDevice->ReleaseResource(m_resource);
		m_resource = nullptr;
	}

	if (allowCpuWrites)
		m_usageState = ResourceState::GenericRead;

	const bool isConstantBuffer = HasFlag(m_type, ResourceType::ConstantBuffer);
	if (isConstantBuffer)
	{
		elementSize = Math::AlignUp(elementSize, 256);
	}

	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Flags = (isConstantBuffer || allowCpuWrites) ? D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Height = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Width = (UINT64)m_bufferSize;

	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = (isConstantBuffer || allowCpuWrites) ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	assert_succeeded(
		GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
			&desc, GetResourceState(m_usageState), nullptr, IID_PPV_ARGS(&m_resource)) );

	if (initialData)
	{
		CommandContext::InitializeBuffer(*this, initialData, m_bufferSize);
	}

	m_gpuAddress = m_resource->GetGPUVirtualAddress();

#ifdef RELEASE
	(name);
#else
	m_resource->SetName(MakeWStr(name).c_str());
#endif

	CreateDerivedViews();
}


// TODO - Unify this code with VertexBuffer, ConstantBuffer
void IndexBuffer::Update(size_t sizeInBytes, const void* data)
{
	assert(sizeInBytes <= m_bufferSize);

	CD3DX12_RANGE readRange(0, 0);

	// Map uniform buffer and update it
	void* pData = nullptr;
	ThrowIfFailed(m_resource->Map(0, &readRange, &pData));
	memcpy(pData, data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	m_resource->Unmap(0, nullptr);
}


void IndexBuffer::Update(size_t sizeInBytes, size_t offset, const void* data)
{
	assert((sizeInBytes + offset) <= m_bufferSize);

	CD3DX12_RANGE readRange(0, 0);

	// Map uniform buffer and update it
	uint8_t* pData = nullptr;
	ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
	memcpy((void*)(pData + offset), data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	m_resource->Unmap(0, nullptr);
}


void VertexBuffer::Update(size_t sizeInBytes, const void* data)
{
	assert(sizeInBytes <= m_bufferSize);

	CD3DX12_RANGE readRange(0, 0);

	// Map uniform buffer and update it
	void* pData = nullptr;
	ThrowIfFailed(m_resource->Map(0, &readRange, &pData));
	memcpy(pData, data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	m_resource->Unmap(0, nullptr);
}


void VertexBuffer::Update(size_t sizeInBytes, size_t offset, const void* data)
{
	assert((sizeInBytes + offset) <= m_bufferSize);

	CD3DX12_RANGE readRange(0, 0);

	// Map uniform buffer and update it
	uint8_t* pData = nullptr;
	ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
	memcpy((void*)(pData + offset), data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	m_resource->Unmap(0, nullptr);
}


void ConstantBuffer::Update(size_t sizeInBytes, const void* data)
{
	assert(sizeInBytes <= m_bufferSize);

	CD3DX12_RANGE readRange(0, 0);

	// Map uniform buffer and update it
	void* pData = nullptr;
	ThrowIfFailed(m_resource->Map(0, &readRange, &pData));
	memcpy(pData, data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	m_resource->Unmap(0, nullptr);
}


void ConstantBuffer::Update(size_t sizeInBytes, size_t offset, const void* data)
{
	assert((sizeInBytes + offset) <= m_bufferSize);

	CD3DX12_RANGE readRange(0, 0);

	// Map uniform buffer and update it
	uint8_t* pData = nullptr;
	ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
	memcpy((void*)(pData + offset), data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	m_resource->Unmap(0, nullptr);
}


void ReadbackBuffer::Create(const string& name, uint32_t numElements, uint32_t elementSize)
{
	m_resource = nullptr;

	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;
	m_usageState = ResourceState::CopyDest;

	// Create a readback buffer large enough to hold all texel data
	D3D12_HEAP_PROPERTIES heapProps = {};
	heapProps.Type = D3D12_HEAP_TYPE_READBACK;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	// Readback buffers must be 1-dimensional, i.e. "buffer" not "texture2d"
	D3D12_RESOURCE_DESC resourceDesc = {};
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = m_bufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	assert_succeeded(GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&m_resource)));

#ifdef RELEASE
	(name);
#else
	m_resource->SetName(MakeWStr(name).c_str());
#endif
}


void* ReadbackBuffer::Map()
{
	void* mem;
	m_resource->Map(0, &CD3DX12_RANGE(0, m_bufferSize), &mem);
	return mem;
}


void ReadbackBuffer::Unmap()
{
	m_resource->Unmap(0, &CD3DX12_RANGE(0, 0));
}


GpuBuffer::~GpuBuffer()
{
	g_graphicsDevice->ReleaseResource(m_resource);
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
	m_ibv.Create(m_resource, resDesc);

	m_indexSize16 = (m_elementSize == 2);
}


void VertexBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	//m_srv.Create(m_resource, m_type, resDesc);
	//m_uav.Create(m_resource, m_type, resDesc);
	m_vbv.Create(m_resource, resDesc);
}


void ConstantBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_cbv.Create(m_resource, resDesc);
}


void ByteAddressBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_srv.Create(m_resource, m_type, resDesc);
	m_uav.Create(m_resource, m_type, resDesc);
}


void StructuredBuffer::CreateDerivedViews()
{
	BufferViewDesc resDesc = GetDesc();

	m_srv.Create(m_resource, ResourceType::StructuredBuffer, resDesc);
	m_uav.Create(m_resource, ResourceType::StructuredBuffer, resDesc);
	if (HasFlag(m_type, ResourceType::VertexBuffer))
	{
		m_vbv.Create(m_resource, resDesc);
	}

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

	m_srv.Create(m_resource, m_type, resDesc);
	m_uav.Create(m_resource, m_type, resDesc);
}