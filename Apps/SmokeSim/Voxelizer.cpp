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
#include "Graphics\CommonStates.h"


using namespace Kodiak;
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

	InitRootSigs();
	InitPSOs();

	m_initialized = true;
}


void Voxelizer::InitRootSigs()
{
	// Voxelize
	{
		m_voxelizeRootSig.Reset(1, 0);
		m_voxelizeRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
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
		m_voxelizePSO.SetRenderTargetFormat(Format::Unknown, Format::D32_Float_S8_UInt);

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