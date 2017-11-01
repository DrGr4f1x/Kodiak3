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

#include "RenderPass12.h"


using namespace Kodiak;


void RenderPass::AddColorAttachment(Format colorFormat, ResourceState initialState, ResourceState finalState)
{
	AttachmentDesc desc;
	desc.format = colorFormat;
	desc.initialState = initialState;
	desc.finalState = finalState;
	m_colorAttachments.emplace_back(desc);
}


void RenderPass::AddDepthAttachment(Format depthFormat, ResourceState initialState, ResourceState finalState)
{
	m_depthAttachment.format = depthFormat;
	m_depthAttachment.initialState = initialState;
	m_depthAttachment.finalState = finalState;
	m_hasDepthAttachment = true;
}


ResourceState RenderPass::GetFinalColorState(uint32_t index) const
{
	assert(index < (uint32_t)m_colorAttachments.size());
	return m_colorAttachments[index].finalState;
}


ResourceState RenderPass::GetFinalDepthState() const
{
	assert(m_hasDepthAttachment);
	return m_depthAttachment.finalState;
}