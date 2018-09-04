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


#include "ColorBuffer.h"
#include "DepthBuffer.h"


namespace Kodiak
{

class FrameBuffer
{
	friend class GraphicsContext;

public:
	~FrameBuffer() { Destroy(); }

	void Destroy();

	void SetColorBuffer(uint32_t index, ColorBufferPtr buffer);
	void SetDepthBuffer(DepthBufferPtr buffer);

	ColorBufferPtr GetColorBuffer(uint32_t index) const;
	DepthBufferPtr GetDepthBuffer() const;

	uint32_t GetNumColorBuffers() const;

	bool HasDepthBuffer() const { return m_depthBuffer != nullptr; }

	void Finalize() {}

private:
	std::array<ColorBufferPtr, 8> m_colorBuffers;
	DepthBufferPtr m_depthBuffer;
};

using FrameBufferPtr = std::shared_ptr<FrameBuffer>;
using FrameBufferUPtr = std::unique_ptr<FrameBuffer>;

} // namespace Kodiak