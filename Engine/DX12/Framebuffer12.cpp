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

#include "Framebuffer12.h"


using namespace Kodiak;
using namespace std;


void FrameBuffer::Destroy()
{
	for (uint32_t i = 0; i < 8; ++i)
	{
		m_colorBuffers[i].reset();
	}

	m_depthBuffer.reset();
}


void FrameBuffer::SetColorBuffer(uint32_t index, ColorBufferPtr buffer)
{
	assert(index < 8);

	m_colorBuffers[index] = buffer;
}


void FrameBuffer::SetDepthBuffer(DepthBufferPtr buffer)
{
	m_depthBuffer = buffer;
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

	for (uint32_t i = 0; i < 8; ++i)
	{
		count += m_colorBuffers[i] != nullptr ? 1 : 0;
	}

	return count;
}