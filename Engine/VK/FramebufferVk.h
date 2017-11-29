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

#include "ColorBufferVk.h"
#include "DepthBufferVk.h"

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class DepthBuffer;
class RenderPass;

class FrameBuffer
{
public:
	~FrameBuffer() { Destroy(); }

	void Destroy();

	void Create(ColorBufferPtr& rtv, RenderPass& renderpass);
	void Create(ColorBufferPtr& rtv, DepthBufferPtr& dsv, RenderPass& renderpass);

	ColorBufferPtr& GetColorBuffer(uint32_t index);
	DepthBufferPtr& GetDepthBuffer() { return m_depthBuffer; }

	size_t GetNumColorBuffers() const { return m_colorBuffers.size(); }

	// TODO MRTs

	VkFramebuffer GetFramebuffer() { return m_framebuffer; }

	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }

private:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	VkFramebuffer m_framebuffer{ VK_NULL_HANDLE };

	std::vector<ColorBufferPtr> m_colorBuffers;
	DepthBufferPtr m_depthBuffer;
};

using FrameBufferPtr = std::shared_ptr<FrameBuffer>;
using FrameBufferUPtr = std::unique_ptr<FrameBuffer>;

} // namespace Kodiak