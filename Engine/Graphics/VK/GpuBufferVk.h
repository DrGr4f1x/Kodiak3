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


#include "GpuResourceVk.h"


namespace Kodiak
{

// Forward declarations
class CommandContext;


class GpuBuffer : public GpuResource
{
	friend class CommandContext;
	friend class GraphicsContext;
	friend class ComputeContext;

public:
	GpuBuffer() : m_buffer(nullptr) {}
	explicit GpuBuffer(UVkBuffer* ubuffer) : m_buffer(ubuffer) {}
	~GpuBuffer();

	UVkBuffer* GetBufferHandle() { return m_buffer.Get(); }
	const UVkBuffer* GetBufferHandle() const { return m_buffer.Get(); }

	const VkDescriptorBufferInfo* GetBufferInfoPtr() const { return &m_bufferInfo; }
	//const VkDescriptorBufferInfo* GetUAVBufferInfoPtr() const { return &m_bufferInfoUAV; }

	size_t GetSize() const { return m_bufferSize; }
	size_t GetElementCount() const { return m_elementCount; }
	size_t GetElementSize() const { return m_elementSize; }

	// Create a buffer.  If initial data is provided, it will be copied into the buffer using the default command context.
	void Create(const std::string& name, size_t numElements, size_t elementSize, bool allowCpuWrites, const void* initialData = nullptr);

	uint64_t GetGpuAddress() const { return m_gpuAddress; }

protected:
	GpuBuffer(ResourceType type)
	{
		m_usageState = ResourceState::Common;
		m_type = type;
	}
	virtual void CreateDerivedViews() = 0;
	
protected:
	Microsoft::WRL::ComPtr<UVkBuffer> m_buffer;

	size_t m_bufferSize{ 0 };
	size_t m_elementCount{ 0 };
	size_t m_elementSize{ 0 };

	uint64_t m_gpuAddress{ 0 };

	VkDescriptorBufferInfo m_bufferInfo{};
	//VkDescriptorBufferInfo m_bufferInfoUAV{};
};


class IndexBuffer : public GpuBuffer
{
public:
	IndexBuffer() : GpuBuffer(ResourceType::IndexBuffer) {}

	void Update(size_t sizeInBytes, const void* data);
	void Update(size_t sizeInBytes, size_t offset, const void* data);

	bool IndexSize16Bit() const { return m_indexSize16; }

protected:
	void CreateDerivedViews() override;

private:
	bool m_indexSize16{ false };
};


class VertexBuffer : public GpuBuffer
{
public:
	VertexBuffer() : GpuBuffer(ResourceType::VertexBuffer) {}

	void Update(size_t sizeInBytes, const void* data);
	void Update(size_t sizeInBytes, size_t offset, const void* data);

protected:
	void CreateDerivedViews() override {}
};


class ConstantBuffer : public GpuBuffer
{
public:
	ConstantBuffer() : GpuBuffer(ResourceType::ConstantBuffer)
	{
		m_usageState = ResourceState::GenericRead;
	}

	void Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData = nullptr)
	{

#if DX12
		GpuBuffer::Create(name, numElements, elementSize, true);
		if (initialData)
		{
			Update(m_bufferSize, initialData);
		}
#else
		GpuBuffer::Create(name, numElements, elementSize, true, initialData);
#endif
	}

	void Update(size_t sizeInBytes, const void* data);
	void Update(size_t sizeInBytes, size_t offset, const void* data);

protected:
	void CreateDerivedViews() override;
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

	const VkDescriptorBufferInfo* GetCounterSRVBufferInfoPtr(CommandContext& context);
	const VkDescriptorBufferInfo* GetCounterUAVBufferInfoPtr(CommandContext& context);

	void CreateWithFlags(const std::string& name, size_t numElements, size_t elementSize, ResourceType flags, const void* initialData = nullptr)
	{
		m_type |= flags;
		GpuBuffer::Create(name, numElements, elementSize, false, initialData);
	}

protected:
	void CreateDerivedViews();

private:
	ByteAddressBuffer m_counterBuffer;
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