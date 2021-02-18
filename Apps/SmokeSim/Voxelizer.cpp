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

#include "Voxelizer.h"

#include "SmokeSimUtils.h"
#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void Voxelizer::Initialize(ColorBufferPtr obstacleTex3D, ColorBufferPtr obstacleVelocityTex3D)
{
	m_initialized = false;

	m_obstacleTex3D = obstacleTex3D;
	m_obstacleVelocityTex3D = obstacleVelocityTex3D;

	m_width = obstacleTex3D->GetWidth();
	m_height = obstacleTex3D->GetHeight();
	m_depth = obstacleTex3D->GetDepth();

	ComputeFlattened3DTextureDims(m_depth, m_rows, m_cols);

	m_depthBuffer = make_shared<DepthBuffer>();
	m_depthBuffer->Create("Voxelizer Depth Buffer", m_width * m_rows, m_height * m_cols, Format::D32_Float_S8_UInt);

	m_debugColorBuffer = make_shared<ColorBuffer>();
	m_debugColorBuffer->Create("Voxelizer Debug Color Buffer", m_width * m_rows, m_height * m_cols, 1, Format::R8G8B8A8_UNorm);

	InitFrameBuffers();
	InitRootSigs();
	InitPSOs();
	InitSliceVertices();
	InitResources();

	m_initialized = true;
}


void Voxelizer::SetGridToWorldMatrix(const Matrix4& gridToWorldMatrix)
{
	m_gridToWorldMatrix = gridToWorldMatrix;
	m_worldToGridMatrix = Invert(m_gridToWorldMatrix);
}


void Voxelizer::AddModel(ModelPtr model)
{
	Object obj = {};

	obj.model = model;

	m_sceneObjects.push_back(obj);
}


void Voxelizer::Update(float deltaT)
{
	
}


void Voxelizer::Render(GraphicsContext& context)
{
	ScopedDrawEvent event(context, "Voxelize");

	StencilClipScene(context);
	DrawSlices(context);
}


void Voxelizer::InitFrameBuffers()
{
	// Voxelize
	{
		m_voxelizeFBO.SetColorBuffer(0, m_debugColorBuffer);
		m_voxelizeFBO.SetDepthBuffer(m_depthBuffer);
		m_voxelizeFBO.Finalize();
	}

	// Resolve
	{
		m_resolveFBO.SetColorBuffer(0, m_obstacleTex3D);
		m_resolveFBO.Finalize();
	}

	// Gen Velocity
	{
		m_genVelocityFBO.SetColorBuffer(0, m_obstacleVelocityTex3D);
		m_genVelocityFBO.SetColorBuffer(1, m_obstacleTex3D);
		m_genVelocityFBO.Finalize();
	}
}


void Voxelizer::InitRootSigs()
{
	// Voxelize
	{
		m_voxelizeRootSig.Reset(1, 0);
		m_voxelizeRootSig[0].InitAsConstants(0, 16, ShaderVisibility::Vertex);
		m_voxelizeRootSig.Finalize("Voxelize Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
	}

	// Resolve
	{
		m_resolveRootSig.Reset(1, 0);
		m_resolveRootSig[0].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
		m_resolveRootSig.Finalize("Resolve Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
	}

	// Gen Velocity
	{
		m_genVelocityRootSig.Reset(2, 0);
		m_genVelocityRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
		m_genVelocityRootSig[1].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Geometry);
		m_genVelocityRootSig.Finalize("Gen Velocity Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
	}
}


void Voxelizer::InitPSOs()
{
	// Voxelize
	{
		m_voxelizePSO.SetRootSignature(m_voxelizeRootSig);
		m_voxelizePSO.SetRenderTargetFormat(Format::R8G8B8A8_UNorm, Format::D32_Float_S8_UInt);

		m_voxelizePSO.SetBlendState(CommonStates::BlendDisable());
		m_voxelizePSO.SetRasterizerState(CommonStates::RasterizerTwoSided());

		DepthStencilStateDesc depthStencilDesc = {};
		depthStencilDesc.depthEnable = false;
		depthStencilDesc.depthWriteMask = DepthWrite::Zero;
		depthStencilDesc.stencilEnable = true;
		depthStencilDesc.stencilReadMask = 0x00;
		depthStencilDesc.stencilWriteMask = 0xFF;
		depthStencilDesc.frontFace.stencilFunc = ComparisonFunc::Always;
		depthStencilDesc.frontFace.stencilPassOp = StencilOp::Decr;
		depthStencilDesc.frontFace.stencilFailOp = StencilOp::Keep;
		depthStencilDesc.backFace.stencilFunc = ComparisonFunc::Always;
		depthStencilDesc.backFace.stencilPassOp = StencilOp::Incr;
		depthStencilDesc.backFace.stencilFailOp = StencilOp::Keep;
		m_voxelizePSO.SetDepthStencilState(depthStencilDesc);

		m_voxelizePSO.SetPrimitiveTopology(PrimitiveTopology::TriangleStrip);
		m_voxelizePSO.SetPrimitiveRestart(IndexBufferStripCutValue::Value_0xFFFF);

		m_voxelizePSO.SetVertexShader("VoxelizeVS");
		m_voxelizePSO.SetPixelShader("VoxelizePS");

		VertexStreamDesc vertexStream{ 0, 3 * sizeof(float), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 }
		};

		m_voxelizePSO.SetInputLayout(vertexStream, vertexElements);
		m_voxelizePSO.Finalize();
	}

	// Resolve
	{
		m_resolvePSO.SetRootSignature(m_resolveRootSig);
		m_resolvePSO.SetRenderTargetFormat(Format::R8_UNorm, Format::Unknown);

		m_resolvePSO.SetBlendState(CommonStates::BlendDisable());
		m_resolvePSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_resolvePSO.SetDepthStencilState(CommonStates::DepthStateDisabled());

		m_resolvePSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

		m_resolvePSO.SetVertexShader("ResolveVS");
		m_resolvePSO.SetGeometryShader("ResolveGS");
		m_resolvePSO.SetPixelShader("ResolvePS");

		VertexStreamDesc vertexStream{ 0, sizeof(SliceVertex), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(SliceVertex, position), InputClassification::PerVertexData, 0 },
			{ "TEXCOORD", 0, Format::R32G32B32_Float, 0, offsetof(SliceVertex, texcoord), InputClassification::PerVertexData, 0 }
		};

		m_resolvePSO.SetInputLayout(vertexStream, vertexElements);
		m_resolvePSO.Finalize();
	}

	// Gen Velocity
	{
		m_genVelocityPSO.SetRootSignature(m_genVelocityRootSig);
		Format rtvFormats[] = { Format::R16G16B16A16_Float, Format::R8_UNorm };
		m_genVelocityPSO.SetRenderTargetFormats(2, rtvFormats, Format::Unknown);

		m_genVelocityPSO.SetBlendState(CommonStates::BlendDisable());
		m_genVelocityPSO.SetRasterizerState(CommonStates::RasterizerTwoSided());
		m_genVelocityPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());

		m_genVelocityPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleStrip);
		m_genVelocityPSO.SetPrimitiveRestart(IndexBufferStripCutValue::Value_0xFFFF);

		m_genVelocityPSO.SetVertexShader("GenVelocityVS");
		m_genVelocityPSO.SetGeometryShader("GenVelocityGS");
		m_genVelocityPSO.SetPixelShader("GenVelocityPS");

		VertexStreamDesc vertexStream{ 0, 3 * sizeof(float), InputClassification::PerVertexData };
		vector<VertexElementDesc> vertexElements =
		{
			{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 }
		};

		m_genVelocityPSO.SetInputLayout(vertexStream, vertexElements);
		m_genVelocityPSO.Finalize();
	}
}


void Voxelizer::InitSliceVertices()
{
	using namespace Math;

	vector<SliceVertex> vertices;
	vertices.reserve(6 * m_depth);

	for (uint32_t z = 0; z < m_depth; ++z)
	{
		uint32_t row = z / m_cols;
		uint32_t col = z % m_cols;
		float x = float(col) * float(m_width);
		float y = float(row) * float(m_height);

		SliceVertex vtx0 = { { -1.0f,  1.0f, 0.5f }, { x, y, float(z) } };
		SliceVertex vtx1 = { { -1.0f, -1.0f, 0.5f }, { x, y + float(m_height), float(z) } };
		SliceVertex vtx2 = { {  1.0f, -1.0f, 0.5f }, { x + float(m_width), y + float(m_height), float(z) } };
		SliceVertex vtx3 = { {  1.0f,  1.0f, 0.5f }, { x + float(m_width), y, float(z) } };

		vertices.push_back(vtx0);
		vertices.push_back(vtx1);
		vertices.push_back(vtx2);
		vertices.push_back(vtx0);
		vertices.push_back(vtx2);
		vertices.push_back(vtx3);
	}

	m_sliceVertexBuffer.Create("Slice vertex buffer", vertices.size(), sizeof(SliceVertex), false, vertices.data());
}


void Voxelizer::InitResources()
{
	m_resolveResources.Init(&m_resolveRootSig);
	m_resolveResources.SetSRV(0, 0, *m_depthBuffer, false);
	m_resolveResources.Finalize();
}


void Voxelizer::StencilClipScene(GraphicsContext& context)
{
	ScopedDrawEvent event(context, "Stencil clip scene");

	context.TransitionResource(*m_debugColorBuffer, ResourceState::RenderTarget);
	context.TransitionResource(*m_depthBuffer, ResourceState::DepthWrite);
	context.ClearColor(*m_debugColorBuffer);
	context.ClearDepthAndStencil(*m_depthBuffer);

	context.BeginRenderPass(m_voxelizeFBO);

	context.SetRootSignature(m_voxelizeRootSig);
	context.SetPipelineState(m_voxelizePSO);
	context.SetPrimitiveTopology(PrimitiveTopology::TriangleStrip);

	uint32_t x = 0;
	uint32_t y = 0;
	for (uint32_t z = 0; z < m_depth; ++z)
	{
		ScopedDrawEvent innerEvent(context, fmt::format("Slice z = {}", z));

		x = (z % m_cols) * m_width;
		y = (z / m_cols) * m_height;

		context.SetViewport(float(x), float(y), float(m_width), float(m_height));
		context.SetScissor(x, y, x + m_width, y + m_height);

		float zNear = float(z) / float(m_depth) - 0.5f;
		float zFar = 100000.0f;

		Matrix4 projectionMatrix = Matrix4(XMMatrixOrthographicOffCenterRH(-1.0f, 1.0f, -1.0f, 1.0f, zNear, zFar));
		Matrix4 viewMatrix = Matrix4(Vector3(kYUnitVector), Vector3(kZUnitVector), Vector3(kXUnitVector), Vector3(kZero));

		for (const auto& obj : m_sceneObjects)
		{
			Matrix4 objToProjMatrix = projectionMatrix * viewMatrix * m_worldToGridMatrix * obj.model->GetMatrix();
			
			context.SetConstantArray(0, 16, &objToProjMatrix);
			obj.model->RenderPositionOnly(context);
		}
	}

	context.EndRenderPass();
}


void Voxelizer::DrawSlices(GraphicsContext& context)
{
	//ScopedDrawEvent event(context, "Draw slices");

	context.BeginRenderPass(m_resolveFBO);

	context.SetViewportAndScissor(0, 0, m_width, m_height);
	context.SetRootSignature(m_resolveRootSig);
	context.SetPipelineState(m_resolvePSO);

	context.SetResources(m_resolveResources);

	context.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
	context.SetVertexBuffer(0, m_sliceVertexBuffer);
	context.Draw(6 * m_depth);

	context.EndRenderPass();
}