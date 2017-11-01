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

#include "ColorBuffer12.h"
#include "DepthBuffer12.h"

namespace Kodiak
{

// Forward declarations
class RenderPass;

class FrameBuffer
{
public:
	void Destroy();

	void Create(ColorBuffer& rtv, RenderPass& renderpass);
	void Create(ColorBuffer& rtv, DepthBuffer& dsv, RenderPass& renderpass);

	ColorBuffer& GetColorBuffer(uint32_t index);
	DepthBuffer& GetDepthBuffer() { return m_depthBuffer; }

	size_t GetNumColorBuffers() const { return m_colorBuffers.size(); }

	bool HasDepthBuffer() const { return m_hasDepthBuffer; }

private:
	std::vector<ColorBuffer> m_colorBuffers;
	DepthBuffer m_depthBuffer;
	bool m_hasDepthBuffer{ false };
};

} // namespace Kodiak