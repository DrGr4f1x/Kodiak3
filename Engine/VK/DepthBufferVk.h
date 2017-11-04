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

#include "PixelBufferVk.h"

namespace Kodiak
{

class DepthBuffer : public PixelBuffer
{
public:
	DepthBuffer(float clearDepth = 0.0f, uint32_t clearStencil = 0)
		: m_clearDepth(clearDepth)
		, m_clearStencil(clearStencil)
	{}

	~DepthBuffer() { Destroy(); }

	void Destroy();

	// Create a depth buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create(const std::string& name, uint32_t width, uint32_t height, Format format);

	// Create a depth buffer.  Memory will be allocated in ESRAM (on Xbox One).  On Windows,
	// this functions the same as Create() without a video address.
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