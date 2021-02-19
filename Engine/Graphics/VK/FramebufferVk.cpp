//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "FramebufferVk.h"

#include "GraphicsDeviceVk.h"
#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


void FrameBuffer::SetColorBuffer(uint32_t index, ColorBufferPtr buffer)
{
	assert(index < 8);
	assert(m_width == 0 || m_width == buffer->GetWidth());
	assert(m_height == 0 || m_height == buffer->GetHeight());
	assert(m_numSamples == 0 || m_numSamples == buffer->GetNumSamples());

	m_colorBuffers[index] = buffer;

	m_width = buffer->GetWidth();
	m_height = buffer->GetHeight();
	m_numSamples = buffer->GetNumSamples();
}


void FrameBuffer::SetDepthBuffer(DepthBufferPtr buffer)
{
	assert(m_width == 0 || m_width == buffer->GetWidth());
	assert(m_height == 0 || m_height == buffer->GetHeight());
	assert(m_numSamples == 0 || m_numSamples == buffer->GetNumSamples());

	m_depthBuffer = buffer;

	m_width = buffer->GetWidth();
	m_height = buffer->GetHeight();
	m_numSamples = buffer->GetNumSamples();
}


ColorBufferPtr FrameBuffer::GetColorBuffer(uint32_t index) const
{
	assert(index < 8);

	return m_colorBuffers[index];
}


DepthBufferPtr FrameBuffer::GetDepthBuffer() const
{
	return m_depthBuffer;
}


uint32_t FrameBuffer::GetNumColorBuffers() const
{
	uint32_t count = 0;

	for (uint32_t i = 0; i < 8; ++i, ++count)
	{
		if (!m_colorBuffers[i])
			break;
	}

	return count;
}


void FrameBuffer::Finalize()
{
	vector<ColorBufferPtr> colorBuffers;
	colorBuffers.reserve(8);
	for (uint32_t i = 0; i < 8; ++i)
	{
		if (m_colorBuffers[i])
			colorBuffers.push_back(m_colorBuffers[i]);
		else
			break;
	}
	ThrowIfFailed(g_graphicsDevice->CreateRenderPass(colorBuffers, m_depthBuffer, &m_renderPass));
	ThrowIfFailed(g_graphicsDevice->CreateFramebuffer(colorBuffers, m_depthBuffer, m_renderPass->Get(), &m_bImageless, &m_framebuffer));
}