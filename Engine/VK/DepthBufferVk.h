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

#include "PixelBuffer.h"

namespace Kodiak
{

class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float clearDepth = 0.0f, uint32_t clearStencil = 0)
		: m_clearDepth(clearDepth)
		, m_clearStencil(clearStencil)
	{}

	~DepthBuffer();

	void Create(const std::string& name, uint32_t width, uint32_t height, Format format);
	void Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numSamples, Format format);

	// Get pre-created CPU-visible descriptor handles
	const VkImageView& GetDSV() const { return m_dsv; }

private:
	void CreateDerivedViews(Format format);

private:
	float m_clearDepth;
	uint32_t m_clearStencil;

	VkImageView m_dsv{ VK_NULL_HANDLE };
};

using DepthBufferPtr = std::shared_ptr<DepthBuffer>;
using DepthBufferUPtr = std::unique_ptr<DepthBuffer>;

} // namespace Kodiak