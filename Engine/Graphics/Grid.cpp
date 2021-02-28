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

#include "Application.h"
#include "Camera.h"
#include "Color.h"
#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"
#include "Graphics\VertexTypes.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void Grid::Startup()
{
	InitMesh();
	InitRootSig();
	InitPSO();

	// Setup constant buffer
	m_constantBuffer.Create("Constant buffer", 1, sizeof(m_vsConstants));
	m_vsConstants.viewProjectionMatrix = Math::Matrix4(Math::kIdentity);

	InitResourceSet();
}


void Grid::Shutdown() {}


void Grid::Update(const Camera& camera)
{
	m_vsConstants.viewProjectionMatrix = camera.GetViewProjMatrix();
	m_constantBuffer.Update(sizeof(m_vsConstants), &m_vsConstants);
}


void Grid::Render(GraphicsContext& context)
{
	context.BeginEvent("Grid");

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_pso);

	context.SetResources(m_resources);

	context.SetVertexBuffer(0, m_vertexBuffer);
	context.SetIndexBuffer(m_indexBuffer);

	context.DrawIndexed((uint32_t)m_indexBuffer.GetElementCount());

	context.EndEvent();
}


void Grid::InitMesh()
{
	vector<VertexPositionColor> vertices;

	auto InsertVertex = [&vertices](float x, float y, float z, const Color& c)
	{
		VertexPositionColor vert;
		vert.SetPosition(x, y, z);
		vert.SetColor(c);
		vertices.push_back(vert);
	};

	const float width = float(m_width);
	const float height = float(m_height);

	// Horizontal lines
	float zCur = -10.0f;
	for (int j = -m_height; j <= m_height; ++j)
	{
		if (j == 0)
		{
			Color color = DirectX::Colors::WhiteSmoke;

			InsertVertex(-width, 0.0f, zCur, color);
			InsertVertex(0.0f, 0.0f, zCur, color);

			color = DirectX::Colors::Red;

			InsertVertex(0.0f, 0.0f, zCur, color);
			InsertVertex(width + 1.0f, 0.0f, zCur, color);
		}
		else
		{
			Color color = DirectX::Colors::WhiteSmoke;

			InsertVertex(-width, 0.0f, zCur, color);
			InsertVertex( width, 0.0f, zCur, color);
		}
		zCur += 1.0f;
	}

	// Vertical lines
	float xCur = -10.0f;
	for (int j = -m_width; j <= m_width; ++j)
	{
		if (j == 0)
		{
			Color color = DirectX::Colors::WhiteSmoke;

			InsertVertex(xCur, 0.0f, -height, color);
			InsertVertex(xCur, 0.0f, 0.0f, color);

			color = DirectX::Colors::Blue;

			InsertVertex(xCur, 0.0f, 0.0f, color);
			InsertVertex(xCur, 0.0f, height + 1.0f, color);
		}
		else
		{
			Color color = DirectX::Colors::WhiteSmoke;

			InsertVertex(xCur, 0.0f, -height, color);
			InsertVertex(xCur, 0.0f,  height, color);
		}
		xCur += 1.0f;
	}

	Color color = DirectX::Colors::Green;
	InsertVertex(0.0f, 0.0f, 0.0f, color);
	InsertVertex(0.0f, height, 0.0f, color);

	m_vertexBuffer.Create("Vertex buffer", vertices.size(), sizeof(VertexPositionColor), false, vertices.data());

	vector<uint16_t> indices;
	for (size_t i = 0; i < vertices.size(); ++i)
	{
		indices.push_back(uint16_t(i));
	}

	m_indexBuffer.Create("Index buffer", indices.size(), sizeof(uint16_t), false, indices.data());
}


void Grid::InitRootSig()
{
	m_rootSig.Reset(1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig.Finalize("Root sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void Grid::InitPSO()
{
	m_pso.SetRootSignature(m_rootSig);

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadOnlyReversed());

	m_pso.SetVertexShader("GridVS");
	m_pso.SetPixelShader("GridPS");

	Application* app = GetApplication();
	m_pso.SetRenderTargetFormat(app->GetColorFormat(), app->GetDepthFormat());

	m_pso.SetPrimitiveTopology(PrimitiveTopology::LineList);
	m_pso.SetInputLayout(VertexPositionColor::Stream, VertexPositionColor::Layout);

	m_pso.Finalize();
}


void Grid::InitResourceSet()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_constantBuffer);
	m_resources.Finalize();
}