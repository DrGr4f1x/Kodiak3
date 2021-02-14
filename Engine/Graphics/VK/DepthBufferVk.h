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

#include "Graphics\PixelBuffer.h"


namespace Kodiak
{

class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float clearDepth = 0.0f, uint8_t clearStencil = 0);
	~DepthBuffer();

	void Create(const std::string& name, uint32_t width, uint32_t height, Format format);
	void Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numSamples, Format format);

	// Get pre-created CPU - visible descriptor handles
	VkImageView GetDepthStencilImageView() const { return m_imageViewDepthStencil->Get(); }
	VkImageView GetDepthOnlyImageView() const { return m_imageViewDepthOnly->Get(); }
	VkImageView GetStencilOnlyImageView() const { return m_imageViewStencilOnly->Get(); }
	const VkDescriptorImageInfo* GetDepthImageInfoPtr() const { return &m_imageInfoDepth; }
	const VkDescriptorImageInfo* GetStencilImageInfoPtr() const { return &m_imageInfoStencil; }

	float GetClearDepth() const { return m_clearDepth; }
	uint8_t GetClearStencil() const { return m_clearStencil; }

private:
	void CreateDerivedViews();

private:
	Microsoft::WRL::ComPtr<UVkImageView> m_imageViewDepthStencil{ nullptr };
	Microsoft::WRL::ComPtr<UVkImageView> m_imageViewDepthOnly{ nullptr };
	Microsoft::WRL::ComPtr<UVkImageView> m_imageViewStencilOnly{ nullptr };
	VkDescriptorImageInfo m_imageInfoDepth{};
	VkDescriptorImageInfo m_imageInfoStencil{};

	float m_clearDepth;
	uint8_t m_clearStencil;
};

using DepthBufferPtr = std::shared_ptr<DepthBuffer>;

} // namespace Kodiak