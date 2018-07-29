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
	friend class GraphicsContext;

public:
	~FrameBuffer() { Destroy(); }

	void Destroy();

	void SetColorBuffer(uint32_t index, ColorBufferPtr buffer);
	void SetDepthBuffer(DepthBufferPtr buffer);
	void SetResolveBuffer(uint32_t index, ColorBufferPtr buffer);

	ColorBufferPtr GetColorBuffer(uint32_t index) const;

	uint32_t GetNumColorBuffers() const;
	uint32_t GetNumResolveBuffers() const;

	bool HasDepthBuffer() const { return m_depthBuffer != nullptr; }

	void Finalize(RenderPass& renderpass) {}

private:
	std::array<ColorBufferPtr, 8> m_colorBuffers;
	std::array<ColorBufferPtr, 8> m_resolveBuffers;
	DepthBufferPtr m_depthBuffer;
};

using FrameBufferPtr = std::shared_ptr<FrameBuffer>;
using FrameBufferUPtr = std::unique_ptr<FrameBuffer>;

} // namespace Kodiak