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

#include "Grid.h"

#include "SmokeSimUtils.h"
#include "Graphics\CommandContext.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


namespace
{

void InitSlice(float w, float h, uint32_t z, vector<GridVertex>& vertices)
{
	GridVertex temp1{
		.position = Vector3(2.0f / w - 1.0f, -2.0f / h + 1.0f, 0.0f),
		.texcoord = Vector3(1.0f, 1.0f, float(z)) };
	GridVertex temp2{
		.position = Vector3((w - 1.0f) * 2.0f / w - 1.0f, -2.0f / h + 1.0f, 0.0f),
		.texcoord = Vector3((w - 1.0f), 1.0f, float(z)) };
	GridVertex temp3{
		.position = Vector3((w - 1.0f) * 2.0f / w - 1.0f, -(h - 1.0f) * 2.0f / h + 1.0f, 0.0f),
		.texcoord = Vector3((w - 1.0f), (h - 1.0f), float(z)) };
	GridVertex temp4{
		.position = Vector3(2.0f / w - 1.0f, -(h - 1.0f) * 2.0f / h + 1.0f, 0.0f),
		.texcoord = Vector3(1.0f, (h - 1.0f), float(z)) };

	vertices.push_back(temp1);
	vertices.push_back(temp2);
	vertices.push_back(temp3);
	vertices.push_back(temp1);
	vertices.push_back(temp3);
	vertices.push_back(temp4);
}


void InitLine(float w, float h, uint32_t z, float x1, float y1, float x2, float y2, vector<GridVertex>& vertices)
{
	GridVertex temp1{
		.position = Vector3(x1 * 2.0f / w - 1.0f, -y1 * 2.0f / h + 1.0f, 0.5f),
		.texcoord = Vector3(0.0f, 0.0f, float(z)) };
	GridVertex temp2{
		.position = Vector3(x2 * 2.0f / w - 1.0f, -y2 * 2.0f / h + 1.0f, 0.5f),
		.texcoord = Vector3(0.0f, 0.0f, float(z)) };

	vertices.push_back(temp1);
	vertices.push_back(temp2);
}

} // anonymous namespace


void Grid::Initialize(uint32_t width, uint32_t height, uint32_t depth)
{
	m_width = width;
	m_height = height;
	m_depth = depth;

	m_maxDim = max(max(m_width, m_height), m_depth);

	ComputeFlattened3DTextureDims(m_depth, m_rows, m_cols);

	InitVertexBuffers();
}


void Grid::DrawSlices(GraphicsContext& context)
{
	context.SetVertexBuffer(0, m_slicesVertexBuffer);
	context.Draw(uint32_t(m_slicesVertexBuffer.GetElementCount()));
}


void Grid::DrawSlicesToScreen(GraphicsContext& context)
{
	context.SetVertexBuffer(0, m_renderQuadVertexBuffer);
	context.Draw(uint32_t(m_renderQuadVertexBuffer.GetElementCount()));
}


void Grid::DrawBoundaryQuads(GraphicsContext& context)
{
	context.SetVertexBuffer(0, m_boundarySlicesVertexBuffer);
	context.Draw(uint32_t(m_boundarySlicesVertexBuffer.GetElementCount()));
}


void Grid::DrawBoundaryLines(GraphicsContext& context)
{
	context.SetVertexBuffer(0, m_boundaryLinesVertexBuffer);
	context.Draw(uint32_t(m_boundaryLinesVertexBuffer.GetElementCount()));
}


void Grid::InitVertexBuffers()
{
	constexpr uint32_t verticesPerSlice = 6;
	constexpr uint32_t verticesPerLine = 2;
	constexpr uint32_t linesPerSlice = 4;

	const float w = float(m_width);
	const float h = float(m_height);

	// Render quad buffer
	{
		vector<GridVertex> vertices;
		vertices.reserve(verticesPerSlice * m_depth);

		const float width = float(m_cols * m_width);
		const float height = float(m_rows * m_height);

		for (uint32_t z = 0; z < m_depth; ++z)
		{
			uint32_t column = z % m_cols;
			uint32_t row = z / m_cols;
			float px = float(column * m_width);
			float py = float(row * m_height);

			GridVertex temp1{ 
				.position = Vector3(px * 2.0f / width - 1.0f, -(py * 2.0f / height) + 1.0f, 0.0f), 
				.texcoord = Vector3(0.0f, 0.0f, float(z)) };
			GridVertex temp2{
				.position = Vector3((px + w) * 2.0f / width - 1.0f, -(py * 2.0f / height) + 1.0f, 0.0f),
				.texcoord = Vector3(w, 0.0f, float(z)) };
			GridVertex temp3{
				.position = Vector3((px + w) * 2.0f / width - 1.0f, -((py + h) * 2.0f / height) + 1.0f, 0.0f),
				.texcoord = Vector3(w, h, float(z)) };
			GridVertex temp4{
				.position = Vector3(px * 2.0f / width - 1.0f, -((py + h) * 2.0f / height) + 1.0f, 0.0f),
				.texcoord = Vector3(0.0f, h, float(z)) };

			vertices.push_back(temp1);
			vertices.push_back(temp2);
			vertices.push_back(temp3);
			vertices.push_back(temp1);
			vertices.push_back(temp3);
			vertices.push_back(temp4);
		}

		m_renderQuadVertexBuffer.Create("Render Quad Vertex Buffer", vertices.size(), sizeof(GridVertex), vertices.data());
	}

	// Slices buffer
	{
		vector<GridVertex> vertices;
		vertices.reserve(verticesPerSlice * (m_depth - 2));

		for (uint32_t z = 1; z < m_depth - 1; ++z)
		{
			InitSlice(w, h, z, vertices);
		}

		m_slicesVertexBuffer.Create("Slices Vertex Buffer", vertices.size(), sizeof(GridVertex), vertices.data());
	}

	// Boundary slices buffer
	{
		vector<GridVertex> vertices;
		vertices.reserve(verticesPerSlice * 2);

		InitSlice(w, h, 0, vertices);
		InitSlice(w, h, m_depth - 1, vertices);

		m_boundarySlicesVertexBuffer.Create("Boundary Slices Vertex Buffer", vertices.size(), sizeof(GridVertex), vertices.data());
	}

	// Boundary lines buffer
	{
		vector<GridVertex> vertices;
		vertices.reserve(verticesPerSlice * linesPerSlice * m_depth);

		for (uint32_t z = 0; z < m_depth; ++z)
		{
			InitLine(w, h, z, 0.0f, 1.0f, w, 1.0f, vertices);
			InitLine(w, h, z, 0.0f, h, w, h, vertices);
			InitLine(w, h, z, 1.0f, 0.0f, 1.0f, h, vertices);
			InitLine(w, h, z, w, 0.0f, w, h, vertices);
		}

		m_boundaryLinesVertexBuffer.Create("Boundary Lines Vertex Buffer", vertices.size(), sizeof(GridVertex), vertices.data());
	}
}