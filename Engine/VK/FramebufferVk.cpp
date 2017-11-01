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
#include "DepthBufferVk.h"
#include "GraphicsDeviceVk.h"
#include "RenderPassVk.h"


using namespace Kodiak;


void FrameBuffer::Destroy()
{
	vkDestroyFramebuffer(GetDevice(), m_framebuffer, nullptr);
	m_framebuffer = VK_NULL_HANDLE;
}


void FrameBuffer::Create(ColorBuffer& rtv, RenderPass& renderpass)
{
	VkImageView attachment = rtv.GetRTV();

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = nullptr;
	frameBufferCreateInfo.renderPass = renderpass.GetRenderPass();
	frameBufferCreateInfo.attachmentCount = 1;
	frameBufferCreateInfo.pAttachments = &attachment;
	frameBufferCreateInfo.width = rtv.GetWidth();
	frameBufferCreateInfo.height = rtv.GetHeight();
	frameBufferCreateInfo.layers = 1;

	ThrowIfFailed(vkCreateFramebuffer(GetDevice(), &frameBufferCreateInfo, nullptr, &m_framebuffer));
}


void FrameBuffer::Create(ColorBuffer& rtv, DepthBuffer& dsv, RenderPass& renderpass)
{
	VkImageView attachments[2] = { rtv.GetRTV(), dsv.GetDSV() };

	VkFramebufferCreateInfo frameBufferCreateInfo = {};
	frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	frameBufferCreateInfo.pNext = nullptr;
	frameBufferCreateInfo.renderPass = renderpass.GetRenderPass();
	frameBufferCreateInfo.attachmentCount = 2;
	frameBufferCreateInfo.pAttachments = attachments;
	frameBufferCreateInfo.width = rtv.GetWidth();
	frameBufferCreateInfo.height = rtv.GetHeight();
	frameBufferCreateInfo.layers = 1;

	ThrowIfFailed(vkCreateFramebuffer(GetDevice(), &frameBufferCreateInfo, nullptr, &m_framebuffer));
}