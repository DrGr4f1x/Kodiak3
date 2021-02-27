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

#include "Graphics\GpuBuffer.h"

// Forward declarations
namespace Kodiak { class GraphicsContext; }


class Grid
{
public:
	void Initialize(uint32_t width, uint32_t height, uint32_t depth);

	void DrawSlices(Kodiak::GraphicsContext& context);
	void DrawSlicesToScreen(Kodiak::GraphicsContext& context);
	void DrawBoundaryQuads(Kodiak::GraphicsContext& context);
	void DrawBoundaryLines(Kodiak::GraphicsContext& context);

private:
	void InitVertexBuffers();

private:
	// 3D volume dimensions
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_depth{ 0 };
	uint32_t m_maxDim{ 0 };

	// Rows, columns of flattened 3D volume
	uint32_t m_rows{ 0 };
	uint32_t m_cols{ 0 };

	Kodiak::VertexBuffer m_renderQuadVertexBuffer;
	Kodiak::VertexBuffer m_slicesVertexBuffer;
	Kodiak::VertexBuffer m_boundarySlicesVertexBuffer;
	Kodiak::VertexBuffer m_boundaryLinesVertexBuffer;
};