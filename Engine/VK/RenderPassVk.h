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

namespace Kodiak
{

class RenderPass
{
public:
	void Destroy();

	void AddColorAttachment(Format colorFormat, ResourceState initialState, ResourceState finalState);
	void AddDepthAttachment(Format depthFormat, ResourceState initialState, ResourceState finalState);

	void Finalize();

	VkRenderPass GetRenderPass() { return m_renderPass; }

private:
	VkRenderPass m_renderPass{ VK_NULL_HANDLE };
	std::vector<VkAttachmentDescription> m_colorAttachments;
	VkAttachmentDescription m_depthAttachment;
	bool m_hasDepthAttachment{ false };
};

} // namespace Kodiak