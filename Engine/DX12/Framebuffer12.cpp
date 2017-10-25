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


void Framebuffer::Destroy()
{
	for (auto& cb : m_colorBuffers)
	{
		cb.Destroy();
	}
	m_colorBuffers.clear();

	m_depthBuffer.Destroy();
	m_hasDepthBuffer = false;
}


void Framebuffer::Create(ColorBuffer& rtv, RenderPass& renderpass)
{
	m_colorBuffers.push_back(rtv);
	m_hasDepthBuffer = false;
}


void Framebuffer::Create(ColorBuffer& rtv, DepthBuffer& dsv, RenderPass& renderpass)
{
	m_colorBuffers.push_back(rtv);
	m_depthBuffer = dsv;
	m_hasDepthBuffer = true;
}


ColorBuffer& Framebuffer::GetColorBuffer(uint32_t index)
{
	assert(index < (uint32_t)m_colorBuffers.size());
	return m_colorBuffers[index];
}