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

// Forward declarations
class ColorBuffer;
class DepthBuffer;
class RenderPass;

class FrameBuffer
{
public:
	void Destroy();

	void Create(ColorBuffer& rtv, RenderPass& renderpass);
	void Create(ColorBuffer& rtv, DepthBuffer& dsv, RenderPass& renderpass);

	// TODO MRTs

	VkFramebuffer GetFramebuffer() { return m_framebuffer; }

private:
	VkFramebuffer m_framebuffer{ VK_NULL_HANDLE };
};

} // namespace Kodiak