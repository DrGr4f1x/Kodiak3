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


#include "Graphics\ColorBuffer.h"
#include "Graphics\DepthBuffer.h"


namespace Kodiak
{

class FrameBuffer
{
	friend class GraphicsContext;

public:
	void SetColorBuffer(uint32_t index, ColorBufferPtr buffer);
	void SetDepthBuffer(DepthBufferPtr buffer);

	ColorBufferPtr GetColorBuffer(uint32_t index) const;
	DepthBufferPtr GetDepthBuffer() const;

	uint32_t GetNumColorBuffers() const;

	bool HasDepthBuffer() const { return m_depthBuffer != nullptr; }

	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	uint32_t GetNumSamples() const { return m_numSamples; }

	VkFramebuffer GetFramebuffer() { return m_framebuffer->Get(); }
	VkRenderPass GetRenderPass() { return m_renderPass->Get(); }
	bool IsImageless() const { return m_bImageless; }

	void Finalize();

private:
	std::array<ColorBufferPtr, 8> m_colorBuffers;
	DepthBufferPtr m_depthBuffer;

	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_numSamples{ 0 };

	bool m_bImageless{ false };

	Microsoft::WRL::ComPtr<UVkFramebuffer> m_framebuffer;
	Microsoft::WRL::ComPtr<UVkRenderPass> m_renderPass;
};

using FrameBufferPtr = std::shared_ptr<FrameBuffer>;
using FrameBufferUPtr = std::unique_ptr<FrameBuffer>;

} // namespace Kodiak