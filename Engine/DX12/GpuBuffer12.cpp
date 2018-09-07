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

#include "GraphicsDevice.h"

#include "CommandContext12.h"
#include "Util12.h"


using namespace Kodiak;


void GpuBuffer::Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData)
{
	m_resource = nullptr;

	if (m_isConstantBuffer)
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
	desc.Flags = m_isConstantBuffer ? D3D12_RESOURCE_FLAG_NONE : D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
	desc.Format = DXGI_FORMAT_UNKNOWN;
	desc.Height = 1;
	desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	desc.MipLevels = 1;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Width = (UINT64)m_bufferSize;

	D3D12_HEAP_PROPERTIES heapProps;
	heapProps.Type = m_isConstantBuffer ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
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
	byte* pData = nullptr;
	ThrowIfFailed(m_resource->Map(0, &readRange, reinterpret_cast<void**>(&pData)));
	memcpy((void*)(pData + offset), data, sizeInBytes);
	// Unmap after data has been copied
	// Note: Since we requested a host coherent memory type for the uniform buffer, the write is instantly visible to the GPU
	m_resource->Unmap(0, nullptr);
}