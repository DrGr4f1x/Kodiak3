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


using namespace Kodiak;


void GpuBuffer::Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData)
{
	GpuResource::Destroy();

	if (m_forceAlign256)
	{
		elementSize = Math::AlignUp(elementSize, 256);
	}

	m_elementCount = numElements;
	m_elementSize = elementSize;
	m_bufferSize = numElements * elementSize;

	auto resourceDesc = DescribeBuffer();

	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = m_heapType;
	heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	heapProps.CreationNodeMask = 1;
	heapProps.VisibleNodeMask = 1;

	assert_succeeded(
		GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
			&resourceDesc, m_usageState, nullptr, MY_IID_PPV_ARGS(&m_resource)) );

	m_gpuVirtualAddress = m_resource->GetGPUVirtualAddress();

	if (initialData)
	{
		CommandContext::InitializeBuffer(*this, initialData, m_bufferSize);
	}

#ifdef RELEASE
	(name);
#else
	m_resource->SetName(MakeWStr(name).c_str());
#endif

	CreateDerivedViews();
}


D3D12_RESOURCE_DESC GpuBuffer::DescribeBuffer()
{
	assert(m_bufferSize != 0);

	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = 1;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	desc.Flags = m_resourceFlags;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Height = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Width = (UINT64)m_bufferSize;

	return desc;
}


void IndexBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = (UINT)m_bufferSize / 4;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;

	if (m_srv.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_srv = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv);


	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	uavDesc.Buffer.NumElements = (UINT)m_bufferSize / 4;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_RAW;

	if (m_uav.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_uav = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateUnorderedAccessView(m_resource.Get(), nullptr, &uavDesc, m_uav);


	m_ibv.BufferLocation = m_gpuVirtualAddress;
	m_ibv.Format = m_elementSize == sizeof(uint32_t) ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT;
	m_ibv.SizeInBytes = static_cast<UINT>(m_bufferSize);
}


void VertexBuffer::CreateDerivedViews()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = (UINT)m_elementCount;
	srvDesc.Buffer.StructureByteStride = (UINT)m_elementSize;
	srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

	if (m_srv.ptr == D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN)
	{
		m_srv = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}
	GetDevice()->CreateShaderResourceView(m_resource.Get(), &srvDesc, m_srv);


	D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
	uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.Buffer.CounterOffsetInBytes = 0;
	uavDesc.Buffer.NumElements = (UINT)m_elementCount;
	uavDesc.Buffer.StructureByteStride = (UINT)m_elementSize;
	uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

	m_vbv.BufferLocation = m_gpuVirtualAddress;
	m_vbv.SizeInBytes = static_cast<UINT>(m_bufferSize);
	m_vbv.StrideInBytes = (UINT)m_elementSize;
}


void ConstantBuffer::CreateDerivedViews()
{
	UINT size = static_cast<UINT>(Math::AlignUp(m_bufferSize, 16));

	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc;
	cbvDesc.BufferLocation = m_gpuVirtualAddress;
	cbvDesc.SizeInBytes = size;

	m_cbv = AllocateDescriptor(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	GetDevice()->CreateConstantBufferView(&cbvDesc, m_cbv);
}


void ConstantBuffer::Update(size_t sizeInBytes, const void* data)
{
	assert(sizeInBytes <= m_bufferSize);

	// Map uniform buffer and update it
	void* pData = nullptr;
	ThrowIfFailed(m_resource->Map(0, nullptr, &pData));
	memcpy(pData, data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	m_resource->Unmap(0, nullptr);
}