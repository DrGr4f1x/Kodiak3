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

#include "Graphics\Framebuffer.h"

#include "Graphics\GraphicsDevice.h"

#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


void FrameBuffer::Finalize()
{
	VkRenderPass renderpass{ VK_NULL_HANDLE };
	VkFramebuffer framebuffer{ VK_NULL_HANDLE };

	uint32_t rtCount = 0;
	const uint32_t numColorBuffers = GetNumColorBuffers();

	// Create renderpass
	{
		VkRenderPassCreateInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rpInfo.pNext = 0;
		rpInfo.flags = 0;

		vector<VkAttachmentDescription> attachments(numColorBuffers + 1);
		vector<uint32_t> regToAttachmentIndex(attachments.size(), VK_ATTACHMENT_UNUSED);

		// Process color targets
		for (uint32_t i = 0; i < numColorBuffers; ++i)
		{
			auto format = m_colorBuffers[i]->GetFormat();
			if (format != Format::Unknown)
			{
				auto& desc = attachments[rtCount];
				regToAttachmentIndex[i] = rtCount;
				++rtCount;

				desc.flags = 0;
				desc.format = static_cast<VkFormat>(format);
				desc.samples = SamplesToFlags(m_numSamples);
				desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // This is a color attachment
				desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // This is a color attachment
				desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
		}

		bool hasColor = rtCount > 0;
		bool hasDepth = false;

		// Process depth target
		if (m_depthBuffer && m_depthBuffer->GetFormat() != Format::Unknown)
		{
			auto& depthDesc = attachments[rtCount];
			regToAttachmentIndex.back() = rtCount;
			rtCount++;

			depthDesc.flags = 0;
			depthDesc.format = static_cast<VkFormat>(m_depthBuffer->GetFormat());
			depthDesc.samples = SamplesToFlags(m_numSamples);
			depthDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			hasDepth = true;
		}

		// Init subpass
		VkSubpassDescription subpassDesc = {};
		subpassDesc.flags = 0;

		vector<VkAttachmentReference> attachmentRefs(attachments.size());

		if (hasDepth)
		{
			auto& depthRef = attachmentRefs.back();
			depthRef.attachment = regToAttachmentIndex.back();
			depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			subpassDesc.pDepthStencilAttachment = &attachmentRefs.back();
		}

		if (hasColor)
		{
			for (uint32_t i = 0; i < numColorBuffers; ++i)
			{
				auto& ref = attachmentRefs[i];
				ref.attachment = regToAttachmentIndex[i];
				ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			subpassDesc.colorAttachmentCount = numColorBuffers;
			subpassDesc.pColorAttachments = attachmentRefs.data();
		}

		// Build renderpass
		VkRenderPassCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.attachmentCount = rtCount;
		createInfo.pAttachments = attachments.data();
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpassDesc;
		createInfo.dependencyCount = 0;
		createInfo.pDependencies = nullptr;

		ThrowIfFailed(vkCreateRenderPass(GetDevice(), &createInfo, nullptr, &renderpass));
	}

	// Create framebuffer
	{
		vector<VkImageView> attachments(rtCount);

		uint32_t attachmentCount = 0;
		for (uint32_t i = 0; i < numColorBuffers; ++i)
		{
			attachments[i] = m_colorBuffers[i]->GetRTV().GetHandle();
			++attachmentCount;
		}

		if (m_depthBuffer)
		{
			attachments[attachmentCount] = m_depthBuffer->GetDSV().GetHandle();
			++attachmentCount;
		}

		VkFramebufferCreateInfo frameBufferCreateInfo = {};
		frameBufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		frameBufferCreateInfo.pNext = nullptr;
		frameBufferCreateInfo.renderPass = renderpass;
		frameBufferCreateInfo.attachmentCount = attachmentCount;
		frameBufferCreateInfo.pAttachments = attachments.data();
		frameBufferCreateInfo.width = m_width;
		frameBufferCreateInfo.height = m_height;
		frameBufferCreateInfo.layers = 1;

		ThrowIfFailed(vkCreateFramebuffer(GetDevice(), &frameBufferCreateInfo, nullptr, &framebuffer));
	}

	m_handle = FboHandle::Create(framebuffer, renderpass);
}