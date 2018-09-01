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


namespace Kodiak
{

class GpuBuffer : public GpuResource
{
public:
	virtual ~GpuBuffer() { Destroy(); }
	void Destroy();

	VkBuffer GetBuffer() { return m_buffer; }
	const VkBuffer GetBuffer() const { return m_buffer; }

	VkDescriptorBufferInfo& GetDescriptorInfo() { return m_descriptorInfo; }
	const VkDescriptorBufferInfo& GetDescriptorInfo() const { return m_descriptorInfo; }

	size_t GetSize() const { return m_bufferSize; }
	size_t GetElementCount() const { return m_elementCount; }
	size_t GetElementSize() const { return m_elementSize; }

protected:
	void CreateBuffer(const std::string& name, size_t numElements, size_t elementSize, VkBufferUsageFlagBits flags, bool bHostMappable, const void* initialData);

protected:
	size_t		m_bufferSize{ 0 };
	size_t		m_elementCount{ 0 };
	size_t		m_elementSize{ 0 };

	VkBuffer		m_buffer{ VK_NULL_HANDLE };
	VkDescriptorBufferInfo m_descriptorInfo;
};


class IndexBuffer : public GpuBuffer
{
public:
	void Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData = nullptr);

	VkIndexType GetIndexType() const { return m_indexType; }

private:
	VkIndexType m_indexType{ VK_INDEX_TYPE_UINT16 };
};


class VertexBuffer : public GpuBuffer
{
public:
	void Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData = nullptr);
};


class ConstantBuffer : public GpuBuffer
{
public:
	void Create(const std::string& name, size_t numElements, size_t elementSize, const void* initialData = nullptr);

	void Update(size_t sizeInBytes, const void* data);
	void Update(size_t sizeInBytes, size_t offset, const void* data);

	const VkDescriptorBufferInfo& GetCBV() const { return m_descriptorInfo; }
};

} // namespace Kodiak