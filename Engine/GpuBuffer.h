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

#include "GpuResource.h"
#include "ResourceView.h"

namespace Kodiak
{

class GpuBuffer : public GpuResource
{
public:
	~GpuBuffer();

	const ShaderResourceView& GetSRV() const { return m_srv; }
	const UnorderedAccessView& GetUAV() const { return m_uav; }

	size_t GetSize() const { return m_bufferSize; }
	size_t GetElementCount() const { return m_elementCount; }
	size_t GetElementSize() const { return m_elementSize; }

	// Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
	void Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData = nullptr);

	uint64_t GetGpuAddress() const { return m_gpuAddress; }

protected:
	GpuBuffer()
	{
		m_usageState = ResourceState::Common;
		m_type = ResourceType::GenericBuffer;
	}
	virtual void CreateDerivedViews() = 0;

protected:
	size_t m_bufferSize{ 0 };
	size_t m_elementCount{ 0 };
	size_t m_elementSize{ 0 };

	uint64_t m_gpuAddress{ 0 };

	// TODO Hacky, rework this
	bool m_isConstantBuffer{ false };

	ShaderResourceView m_srv;
	UnorderedAccessView m_uav;
};


class IndexBuffer : public GpuBuffer
{
public:
	IndexBuffer() : GpuBuffer()
	{
		m_type = ResourceType::IndexBuffer;
	}

	const IndexBufferView& GetIBV() const { return m_ibv; }

	bool IndexSize16Bit() const { return m_indexSize16; }

protected:
	void CreateDerivedViews() override;

private:
	IndexBufferView m_ibv;
	bool m_indexSize16{ false };
};


class VertexBuffer : public GpuBuffer
{
public:
	VertexBuffer() : GpuBuffer()
	{
		m_type = ResourceType::VertexBuffer;
	}

	const VertexBufferView& GetVBV() const { return m_vbv; }

protected:
	void CreateDerivedViews() override;

private:
	VertexBufferView m_vbv;
};


class ConstantBuffer : public GpuBuffer
{
public:
	ConstantBuffer() : GpuBuffer()
	{
		m_isConstantBuffer = true;
		m_usageState = ResourceState::GenericRead;
	}

	const ConstantBufferView& GetCBV() const { return m_cbv; }

	void Update(size_t sizeInBytes, const void* data);
	void Update(size_t sizeInBytes, size_t offset, const void* data);

protected:
	void CreateDerivedViews() override;

private:
	ConstantBufferView m_cbv;
};

} // namespace Kodiak