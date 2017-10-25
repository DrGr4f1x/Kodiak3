//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

#include "GpuResource12.h"

namespace Kodiak
{

class GpuBuffer : public GpuResource
{
public:
	virtual ~GpuBuffer() { Destroy(); }

	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV() const { return m_uav; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV() const { return m_srv; }

	size_t GetSize() const { return m_bufferSize; }
	size_t GetElementCount() const { return m_elementCount; }
	size_t GetElementSize() const { return m_elementSize; }

	// Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
	void Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData = nullptr);

	D3D12_GPU_VIRTUAL_ADDRESS RootConstantBufferView() const { return m_gpuVirtualAddress; }

protected:
	GpuBuffer()
	{
		m_resourceFlags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		m_usageState = D3D12_RESOURCE_STATE_COMMON;
		m_uav.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_srv.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}

	D3D12_RESOURCE_DESC DescribeBuffer();
	virtual void CreateDerivedViews() = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE m_uav;
	D3D12_CPU_DESCRIPTOR_HANDLE m_srv;

	size_t m_bufferSize{ 0 };
	size_t m_elementCount{ 0 };
	size_t m_elementSize{ 0 };
	bool m_forceAlign256{ false };

	D3D12_RESOURCE_FLAGS m_resourceFlags;
	D3D12_HEAP_TYPE m_heapType{ D3D12_HEAP_TYPE_DEFAULT };
};


class IndexBuffer : public GpuBuffer
{
public:
	const D3D12_INDEX_BUFFER_VIEW& GetIBV() const { return m_ibv; }

private:
	void CreateDerivedViews() override;

private:
	D3D12_INDEX_BUFFER_VIEW m_ibv;
};


class VertexBuffer : public GpuBuffer
{
public:
	const D3D12_VERTEX_BUFFER_VIEW& GetVBV() const { return m_vbv; }

private:
	void CreateDerivedViews() override;

private:
	D3D12_VERTEX_BUFFER_VIEW m_vbv;
};


class ConstantBuffer : public GpuBuffer
{
public:
	ConstantBuffer()
	{
		m_usageState = D3D12_RESOURCE_STATE_GENERIC_READ;
		m_heapType = D3D12_HEAP_TYPE_UPLOAD;
		m_resourceFlags = D3D12_RESOURCE_FLAG_NONE;
		m_forceAlign256 = true;
	}

	void CreateDerivedViews() override;

	void Update(size_t sizeInBytes, const void* data);

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_cbv;
};

} // namespace Kodiak