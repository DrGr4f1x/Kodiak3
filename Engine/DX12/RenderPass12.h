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
	friend class GraphicsPSO;
public:
	void Destroy() {}

	void AddColorAttachment(Format colorFormat, ResourceState initialState, ResourceState finalState);
	void AddDepthAttachment(Format depthFormat, ResourceState initialState, ResourceState finalState);

	Format GetColorFormat(uint32_t index) const { return m_colorAttachments[index].format; }
	Format GetDepthFormat() const { return m_depthAttachment.format; }

	ResourceState GetFinalColorState(uint32_t index) const;
	ResourceState GetFinalDepthState() const;

	size_t GetNumColorAttachments() const { return m_colorAttachments.size(); }
	bool HasDepthAttachment() const { return m_hasDepthAttachment; }

	void Finalize() {}

private:
	struct AttachmentDesc
	{
		Format format{ Format::Unknown };
		ResourceState initialState;
		ResourceState finalState;
	};

	std::vector<AttachmentDesc> m_colorAttachments;
	AttachmentDesc m_depthAttachment;
	bool m_hasDepthAttachment{ false };
};

} // namespace Kodiak