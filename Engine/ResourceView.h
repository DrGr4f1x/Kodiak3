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

namespace Kodiak
{

struct ResourceViewDesc
{
	Format format;
	uint32_t arraySize;
	uint32_t firstArraySlice;
	uint32_t mipCount;
	uint32_t mipLevel;
	uint32_t bufferSize;
	uint32_t elementCount;
	uint32_t elementSize;
};


ResourceViewDesc DescribeIndexBuffer(size_t elementSize, size_t bufferSize);
ResourceViewDesc DescribeVertexBuffer(size_t elementSize, size_t elementCount, size_t bufferSize);
ResourceViewDesc DescribeConstantBuffer(size_t bufferSize);


class ShaderResourceView
{
public:
	ShaderResourceView();

	void Create(const ResourceHandle& resource, ResourceType type, const ResourceViewDesc& desc);

	const SrvHandle& GetHandle() const { return m_handle; }

private:
	SrvHandle m_handle;
};


class UnorderedAccessView
{
public:
	UnorderedAccessView();

	void Create(const ResourceHandle& resource, ResourceType type, const ResourceViewDesc& desc);

	const UavHandle& GetHandle() const { return m_handle; }

private:
	UavHandle m_handle;
};


class IndexBufferView
{
public:
	IndexBufferView();

	void Create(const ResourceHandle& resource, const ResourceViewDesc& desc);

	const IbvHandle& GetHandle() const { return m_handle; }

private:
	IbvHandle m_handle;
};


class VertexBufferView
{
public:
	VertexBufferView();

	void Create(const ResourceHandle& resource, const ResourceViewDesc& desc);

	const VbvHandle& GetHandle() const { return m_handle; }

private:
	VbvHandle m_handle;
};


class ConstantBufferView
{
public:
	ConstantBufferView();

	void Create(const ResourceHandle& resource, const ResourceViewDesc& desc);

	const CbvHandle& GetHandle() const { return m_handle; }

private:
	CbvHandle m_handle;
};

} // namespace Kodiak