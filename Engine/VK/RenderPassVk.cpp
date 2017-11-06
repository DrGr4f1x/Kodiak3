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

#include "RenderPassVk.h"

#include "GraphicsDeviceVk.h"


using namespace Kodiak;
using namespace std;


namespace
{

VkImageLayout GetImageLayout(ResourceState state)
{
	switch (state)
	{
	case ResourceState::Undefined:
		return VK_IMAGE_LAYOUT_UNDEFINED;
	case ResourceState::Present:
		return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	case ResourceState::DepthWrite:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	case ResourceState::DepthRead:
		return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
	case ResourceState::PixelShaderResource:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	case ResourceState::NonPixelShaderResource:
		return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	default:
		assert(false);
		return VK_IMAGE_LAYOUT_UNDEFINED;
	}
}

} // anonymous namespace


void RenderPass::Destroy()
{
	vkDestroyRenderPass(GetDevice(), m_renderPass, nullptr);
	m_renderPass = VK_NULL_HANDLE;
}


void RenderPass::AddColorAttachment(Format colorFormat, ResourceState initialState, ResourceState finalState)
{
	VkAttachmentDescription attachment = {};

	attachment.flags = 0;
	attachment.format = static_cast<VkFormat>(colorFormat);
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachment.initialLayout = GetImageLayout(initialState);
	attachment.finalLayout = GetImageLayout(finalState);

	m_colorAttachments.emplace_back(attachment);
}


void RenderPass::AddDepthAttachment(Format colorFormat, ResourceState initialState, ResourceState finalState)
{
	m_depthAttachment.flags = 0;
	m_depthAttachment.format = static_cast<VkFormat>(colorFormat);
	m_depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	m_depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	m_depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	m_depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	m_depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	m_depthAttachment.initialLayout = GetImageLayout(initialState);
	m_depthAttachment.finalLayout = GetImageLayout(finalState);

	m_hasDepthAttachment = true;
}


void RenderPass::Finalize()
{
	vector<VkAttachmentDescription> attachments(m_colorAttachments.begin(), m_colorAttachments.end());
	if (m_hasDepthAttachment)
	{
		attachments.emplace_back(m_depthAttachment);
	}

	// TODO Handle subpasses
	VkAttachmentReference colorReference = {};
	colorReference.attachment = 0;
	colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthReference = {};
	depthReference.attachment = 1;
	depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorReference;
	subpassDescription.pDepthStencilAttachment = m_hasDepthAttachment ? &depthReference : nullptr;
	subpassDescription.inputAttachmentCount = 0;
	subpassDescription.pInputAttachments = nullptr;
	subpassDescription.preserveAttachmentCount = 0;
	subpassDescription.pPreserveAttachments = nullptr;
	subpassDescription.pResolveAttachments = nullptr;

	// TODO Handle dependencies
	// Subpass dependencies for layout transitions
	array<VkSubpassDependency, 2> dependencies;

	dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[0].dstSubpass = 0;
	dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	dependencies[1].srcSubpass = 0;
	dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
	dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
	dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
	dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpassDescription;
	renderPassInfo.dependencyCount = static_cast<uint32_t>(dependencies.size());
	renderPassInfo.pDependencies = dependencies.data();

	ThrowIfFailed(vkCreateRenderPass(GetDevice(), &renderPassInfo, nullptr, &m_renderPass));
}