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
	friend class GraphicsContext;

public:
	void Destroy() {}

	void SetColorAttachment(uint32_t index, Format colorFormat, ResourceState initialState, ResourceState finalState);
	void SetDepthAttachment(Format depthFormat, ResourceState initialState, ResourceState finalState);
	void SetResolveAttachment(uint32_t index, Format colorFormat, ResourceState initialState, ResourceState finalState);

	uint32_t GetNumColorAttachments() const;
	uint32_t GetNumResolveAttachments() const;

	void Finalize() {}

private:
	struct AttachmentDesc
	{
		Format format{ Format::Unknown };
		ResourceState initialState;
		ResourceState finalState;
		bool isValid{ false };
	};

	std::array<AttachmentDesc, 8> m_colorAttachments;
	std::array<AttachmentDesc, 8> m_resolveAttachments;
	AttachmentDesc m_depthAttachment;
};

} // namespace Kodiak