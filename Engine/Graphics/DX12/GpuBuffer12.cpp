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


void GpuBuffer::Create(const string& name, size_t numElements, size_t elementSize, bool allowCpuWrites, const void* initialData)
{
	if (m_resource)
	{
		g_graphicsDevice->ReleaseResource(m_resource.Get());
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

	SetDebugName(m_resource.Get(), name);

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

	SetDebugName(m_resource.Get(), name);
}


void* ReadbackBuffer::Map()
{
	void* mem;
	auto range = CD3DX12_RANGE(0, m_bufferSize);
	m_resource->Map(0, &range, &mem);
	return mem;
}


void ReadbackBuffer::Unmap()
{
	auto range = CD3DX12_RANGE(0, 0);
	m_resource->Unmap(0, &range);
}


GpuBuffer::~GpuBuffer()
{
	g_graphicsDevice->ReleaseResource(m_resource.Get());
}


void IndexBuffer::CreateDerivedViews()
{
	m_indexSize16 = (m_elementSize == 2);

	m_ibvHandle.BufferLocation = m_gpuAddress;
	m_ibvHandle.Format = m_indexSize16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	m_ibvHandle.SizeInBytes = (UINT)m_bufferSize;
}


void VertexBuffer::CreateDerivedViews()
{
	m_vbvHandle.BufferLocation = m_gpuAddress;
	m_vbvHandle.SizeInBytes = (UINT)m_bufferSize;
	m_vbvHandle.StrideInBytes = (UINT)m_elementSize;
}


void ConstantBuffer::CreateDerivedViews()
{
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = m_gpuAddress;
	cbvDesc.SizeInBytes = (UINT)m_bufferSize;

	m_cbvHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	GetDevice()->CreateConstantBufferView(&cbvDesc, m_cbvHandle);
}


void ByteAddressBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = (UINT)m_bufferSize / 4;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

	if (m_srvHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_srvHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srvHandle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.Buffer.NumElements = (UINT)m_bufferSize / 4;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	if (m_uavHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_uavHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateUnorderedAccessView(m_resource.Get(), nullptr, &uavDesc, m_uavHandle);
}


void StructuredBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = (UINT)m_elementCount;
	srvDesc.Buffer.StructureByteStride = (UINT)m_elementSize;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (m_srvHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_srvHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srvHandle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.NumElements = (UINT)m_elementCount;
	uavDesc.Buffer.StructureByteStride = (UINT)m_elementSize;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	m_counterBuffer.Create("StructuredBuffer::Counter", 1, 4, false);

	if (m_uavHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_uavHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateUnorderedAccessView(m_resource.Get(), m_counterBuffer.GetResource(), &uavDesc, m_uavHandle);

	if (HasFlag(m_type, ResourceType::VertexBuffer))
	{
		m_vbvHandle.BufferLocation = m_gpuAddress;
		m_vbvHandle.SizeInBytes = (UINT)m_bufferSize;
		m_vbvHandle.StrideInBytes = (UINT)m_elementSize;
	}
}


const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterSRV(CommandContext& context)
{
	context.TransitionResource(m_counterBuffer, ResourceState::GenericRead);
	return m_counterBuffer.GetSRV();
}


const D3D12_CPU_DESCRIPTOR_HANDLE& StructuredBuffer::GetCounterUAV(CommandContext& context)
{
	context.TransitionResource(m_counterBuffer, ResourceState::UnorderedAccess);
	return m_counterBuffer.GetUAV();
}


void TypedBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = static_cast<DXGI_FORMAT>(m_dataFormat);
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = (UINT)m_elementCount;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (m_srvHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_srvHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srvHandle);

	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = static_cast<DXGI_FORMAT>(m_dataFormat);;
	uavDesc.Buffer.NumElements = (UINT)m_elementCount;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	if (m_uavHandle.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_uavHandle = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateUnorderedAccessView(m_resource.Get(), nullptr, &uavDesc, m_uavHandle);
}