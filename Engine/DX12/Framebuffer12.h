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
	~FrameBuffer() { Destroy(); }

	void Destroy();

	void Create(ColorBufferPtr& rtv, RenderPass& renderpass);
	void Create(ColorBufferPtr& rtv, DepthBufferPtr& dsv, RenderPass& renderpass);

	ColorBufferPtr& GetColorBuffer(uint32_t index);
	DepthBufferPtr& GetDepthBuffer() { return m_depthBuffer; }

	size_t GetNumColorBuffers() const { return m_colorBuffers.size(); }

	bool HasDepthBuffer() const { return m_hasDepthBuffer; }

private:
	std::vector<ColorBufferPtr> m_colorBuffers;
	DepthBufferPtr m_depthBuffer;
	bool m_hasDepthBuffer{ false };
};

using FrameBufferPtr = std::shared_ptr<FrameBuffer>;
using FrameBufferUPtr = std::unique_ptr<FrameBuffer>;

} // namespace Kodiak