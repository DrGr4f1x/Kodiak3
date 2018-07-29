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


void RenderPass::SetColorAttachment(uint32_t index, Format colorFormat, ResourceState initialState, ResourceState finalState)
{
	assert(index < 8);

	m_colorAttachments[index].format = colorFormat;
	m_colorAttachments[index].initialState = initialState;
	m_colorAttachments[index].finalState = finalState;
	m_colorAttachments[index].isValid = true;
}


void RenderPass::SetDepthAttachment(Format depthFormat, ResourceState initialState, ResourceState finalState)
{
	m_depthAttachment.format = depthFormat;
	m_depthAttachment.initialState = initialState;
	m_depthAttachment.finalState = finalState;
	m_depthAttachment.isValid = true;
}


void RenderPass::SetResolveAttachment(uint32_t index, Format colorFormat, ResourceState initialState, ResourceState finalState)
{
	assert(index < 8);

	m_resolveAttachments[index].format = colorFormat;
	m_resolveAttachments[index].initialState = initialState;
	m_resolveAttachments[index].finalState = finalState;
	m_resolveAttachments[index].isValid = true;
}


uint32_t RenderPass::GetNumColorAttachments() const
{
	uint32_t count = 0;

	for (uint32_t i = 0; i < 8; ++i)
	{
		count += m_colorAttachments[i].isValid ? 1 : 0;
	}

	return count;
}


uint32_t RenderPass::GetNumResolveAttachments() const
{
	uint32_t count = 0;

	for (uint32_t i = 0; i < 8; ++i)
	{
		count += m_resolveAttachments[i].isValid ? 1 : 0;
	}

	return count;
}