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

#include "ColorBufferVk.h"
#include "GraphicsDevice.h"
#include "RenderPassVk.h"


using namespace Kodiak;


void FrameBuffer::Destroy()
{
	vkDestroyFramebuffer(GetDevice(), m_framebuffer, nullptr);
	m_framebuffer = VK_NULL_HANDLE;
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


void FrameBuffer::SetResolveBuffer(uint32_t index, ColorBufferPtr buffer)
{
	assert(index < 8);

	m_resolveBuffers[index] = buffer;
}


uint32_t FrameBuffer::GetWidth() const
{
	return m_colorBuffers[0]->GetWidth();
}


uint32_t FrameBuffer::GetHeight() const
{
	return m_colorBuffers[0]->GetHeight();
}


ColorBufferPtr FrameBuffer::GetColorBuffer(uint32_t index) const
{
	assert(index < 8);

	return m_colorBuffers[index];
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


uint32_t FrameBuffer::GetNumResolveBuffers() const
{
	uint32_t count = 0;

	for (uint32_t i = 0; i < 8; ++i)
	{
		count += m_resolveBuffers[i] != nullptr ? 1 : 0;
	}

	return count;
}


void FrameBuffer::Finalize(RenderPass& renderPass)
{
	std::array<VkImageView, 9> attachments;
	int curAttachment = 0;

	uint32_t width = 0;
	uint32_t height = 0;

	for (int i = 0; i < 8; ++i)
	{
		if (m_colorBuffers[i])
		{
			attachments[curAttachment] = m_colorBuffers[i]->GetRTV();
			++curAttachment;
			width = m_colorBuffers[i]->GetWidth();
			height = m_colorBuffers[i]->GetHeight();
		}
	}

	if (m_depthBuffer)
	{
		attachments[curAttachment] = m_depthBuffer->GetDSV().GetHandle();
		++curAttachment;
	}

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = nullptr;
	frameBufferCreateInfo.renderPass = renderPass.GetRenderPass();
	frameBufferCreateInfo.attachmentCount = curAttachment;
	frameBufferCreateInfo.pAttachments = &attachments[0];
	frameBufferCreateInfo.width = width;
	frameBufferCreateInfo.height = height;
	frameBufferCreateInfo.layers = 1;

	ThrowIfFailed(vkCreateFramebuffer(GetDevice(), &frameBufferCreateInfo, nullptr, &m_framebuffer));
}