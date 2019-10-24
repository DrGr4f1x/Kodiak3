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
#include "GraphicsEnums.h"
#include "ResourceView.h"


namespace Kodiak
{

// Forward declarations
class CommandContext;


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
	GpuBuffer(ResourceType type)
	{
		m_usageState = ResourceState::Common;
		m_type = type;
	}
	virtual void CreateDerivedViews() = 0;
	BufferViewDesc GetDesc() const;

protected:
	size_t m_bufferSize{ 0 };
	size_t m_elementCount{ 0 };
	size_t m_elementSize{ 0 };

	uint64_t m_gpuAddress{ 0 };

	ShaderResourceView m_srv;
	UnorderedAccessView m_uav;
};


class IndexBuffer : public GpuBuffer
{
public:
	IndexBuffer() : GpuBuffer(ResourceType::IndexBuffer) {}

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
	VertexBuffer() : GpuBuffer(ResourceType::VertexBuffer) {}

	const VertexBufferView& GetVBV() const { return m_vbv; }

protected:
	void CreateDerivedViews() override;

private:
	VertexBufferView m_vbv;
};


class ConstantBuffer : public GpuBuffer
{
public:
	ConstantBuffer() : GpuBuffer(ResourceType::ConstantBuffer)
	{
		m_usageState = ResourceState::GenericRead;
	}

#if DX12
	void Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData = nullptr)
	{
		GpuBuffer::Create(name, numElements, elementSize);
		if (initialData)
		{
			Update(m_bufferSize, initialData);
		}
	}
#endif

	const ConstantBufferView& GetCBV() const { return m_cbv; }

	void Update(size_t sizeInBytes, const void* data);
	void Update(size_t sizeInBytes, size_t offset, const void* data);

protected:
	void CreateDerivedViews() override;

private:
	ConstantBufferView m_cbv;
};


class ByteAddressBuffer : public GpuBuffer
{
public:
	ByteAddressBuffer() : GpuBuffer(ResourceType::ByteAddressBuffer) {}

protected:
	void CreateDerivedViews() override;
};


class IndirectArgsBuffer : public ByteAddressBuffer
{
public:
	IndirectArgsBuffer()
		: ByteAddressBuffer()
	{
		m_type = ResourceType::IndirectArgsBuffer;
	}
};


class StructuredBuffer : public GpuBuffer
{
public:
	StructuredBuffer() : GpuBuffer(ResourceType::StructuredBuffer) {}

	ByteAddressBuffer& GetCounterBuffer() { return m_counterBuffer; }

	const ShaderResourceView& GetCounterSRV(CommandContext& context);
	const UnorderedAccessView& GetCounterUAV(CommandContext& context);

	const VertexBufferView& GetVBV() const { return m_vbv; }

	void CreateWithFlags(const std::string& name, size_t numElements, size_t elementSize, ResourceType flags, const void* initialData = nullptr)
	{
		m_type |= flags;
		GpuBuffer::Create(name, numElements, elementSize, initialData);
	}

protected:
	void CreateDerivedViews();

private:
	ByteAddressBuffer m_counterBuffer;
	VertexBufferView m_vbv;
};


class TypedBuffer : public GpuBuffer
{
public:
	TypedBuffer(Format format) 
		: GpuBuffer(ResourceType::TypedBuffer)
		, m_dataFormat(format)
	{}

protected:
	void CreateDerivedViews() override;

private:
	Format m_dataFormat;
};


class ReadbackBuffer : public GpuBuffer
{
public:
	ReadbackBuffer() : GpuBuffer(ResourceType::ReadbackBuffer) {}

	void Create(const std::string& name, uint32_t numElements, uint32_t elementSize);

	void* Map();
	void Unmap();

protected:
	void CreateDerivedViews() override {}
};

} // namespace Kodiak