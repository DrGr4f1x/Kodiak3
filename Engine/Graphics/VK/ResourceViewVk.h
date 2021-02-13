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
	ResourceState usage;
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

	void Create(UVkImage* uimage, ResourceType type, const TextureViewDesc& desc);
	void Create(UVkBuffer* ubuffer, ResourceType type, const BufferViewDesc& desc);

	// TODO Get rid of this
	VkImageView& GetImageViewRef() const { return m_imageView->GetRef(); }
	VkImageView GetImageView() const { return m_imageView->Get(); }
	const VkDescriptorImageInfo* GetDescriptorImageInfoPtr() const { return &m_imageInfo; }
	const VkDescriptorBufferInfo* GetDescriptorBufferInfoPtr() const { return &m_bufferInfo; }

private:
	Microsoft::WRL::ComPtr<UVkImageView> m_imageView;
	VkDescriptorImageInfo m_imageInfo{};
	VkDescriptorBufferInfo m_bufferInfo{};
};


class UnorderedAccessView
{
public:
	UnorderedAccessView();

	void Create(UVkImage* uimage, ResourceType type, const TextureViewDesc& desc);
	void Create(UVkBuffer* ubuffer, ResourceType type, const BufferViewDesc& desc);
	void Create(UVkBuffer* ubuffer, ResourceType type, const TypedBufferViewDesc& desc);

	VkImageView GetImageView() const { return m_imageView->Get(); }
	const VkDescriptorImageInfo* GetDescriptorImageInfoPtr() const { return &m_imageInfo; }
	const VkDescriptorBufferInfo* GetDescriptorBufferInfoPtr() const { return &m_bufferInfo; }

private:
	Microsoft::WRL::ComPtr<UVkImageView> m_imageView;
	VkDescriptorImageInfo m_imageInfo{};
	VkDescriptorBufferInfo m_bufferInfo{};
};


class ConstantBufferView
{
public:
	ConstantBufferView();

	void Create(UVkBuffer* ubuffer, const BufferViewDesc& desc);

	const VkDescriptorBufferInfo* GetDescriptorBufferInfoPtr() const { return &m_bufferInfo; }

private:
	VkDescriptorBufferInfo m_bufferInfo{};
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

	void Create(UVkImage* uimage, const DepthStencilViewDesc& desc);

	VkImageView GetImageView() const { return m_imageView->Get(); }

private:
	Microsoft::WRL::ComPtr<UVkImageView> m_imageView;
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

	void Create(UVkImage* uimage, const RenderTargetViewDesc& desc);

	VkImageView GetImageView() const { return m_imageView->Get(); }

private:
	Microsoft::WRL::ComPtr<UVkImageView> m_imageView;
};

} // namespace Kodiak