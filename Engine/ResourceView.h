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

struct TextureViewDesc
{
	Format format;
	uint32_t arraySize;
	uint32_t firstArraySlice;
	uint32_t mipCount;
	uint32_t mipLevel;
	bool isDepth;
	bool isStencil;
};


struct BufferViewDesc
{
	Format format;
	uint32_t bufferSize;
	uint32_t elementCount;
	uint32_t elementSize;
};


struct TypedBufferViewDesc
{
	Format format;
	uint32_t bufferSize;
	uint32_t elementCount;
	uint32_t elementSize;
};


class ShaderResourceView
{
public:
	ShaderResourceView();

	void Create(const ResourceHandle& resource, ResourceType type, const TextureViewDesc& desc);
	void Create(const ResourceHandle& resource, ResourceType type, const BufferViewDesc& desc);

	SrvHandle& GetHandle() { return m_handle; }
	const SrvHandle& GetHandle() const { return m_handle; }

private:
	SrvHandle m_handle;
};


class UnorderedAccessView
{
public:
	UnorderedAccessView();

	void Create(const ResourceHandle& resource, ResourceType type, const TextureViewDesc& desc);
	void Create(const ResourceHandle& resource, ResourceType type, const BufferViewDesc& desc);
	void Create(const ResourceHandle& resource, ResourceType type, const TypedBufferViewDesc& desc);

	UavHandle& GetHandle() { return m_handle; }
	const UavHandle& GetHandle() const { return m_handle; }

private:
	UavHandle m_handle;
};


class IndexBufferView
{
public:
	IndexBufferView();

	void Create(const ResourceHandle& resource, const BufferViewDesc& desc);

	IbvHandle& GetHandle() { return m_handle; }
	const IbvHandle& GetHandle() const { return m_handle; }

private:
	IbvHandle m_handle;
};


class VertexBufferView
{
public:
	VertexBufferView();

	void Create(const ResourceHandle& resource, const BufferViewDesc& desc);

	VbvHandle& GetHandle() { return m_handle; }
	const VbvHandle& GetHandle() const { return m_handle; }

private:
	VbvHandle m_handle;
};


class ConstantBufferView
{
public:
	ConstantBufferView();

	void Create(const ResourceHandle& resource, const BufferViewDesc& desc);

	CbvHandle& GetHandle() { return m_handle; }
	const CbvHandle& GetHandle() const { return m_handle; }

private:
	CbvHandle m_handle;
};


struct DepthStencilViewDesc
{
	Format format;
	bool readOnlyDepth;
	bool readOnlyStencil;
};


class DepthStencilView
{
public:
	DepthStencilView();

	void Create(const ResourceHandle& resource, const DepthStencilViewDesc& desc);

	DsvHandle& GetHandle() { return m_handle; }
	const DsvHandle& GetHandle() const { return m_handle; }

private:
	DsvHandle m_handle;
};


struct RenderTargetViewDesc
{
	Format format;
	uint32_t arraySize;
	uint32_t numMips;
	uint32_t fragmentCount;
	bool nullDesc;
};


class RenderTargetView
{
public:
	RenderTargetView();

	void Create(const ResourceHandle& resource, const RenderTargetViewDesc& desc);

	RtvHandle& GetHandle() { return m_handle; }
	const RtvHandle& GetHandle() const { return m_handle; }

private:
	RtvHandle m_handle;
};

} // namespace Kodiak