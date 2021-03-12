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

#include "VolumeRenderer.h"

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"


using namespace Kodiak;
using namespace std;


void VolumeRenderer::Initialize(uint32_t width, uint32_t height, uint32_t depth)
{
	m_width = width;
	m_height = height;
	m_depth = depth;

	InitRootSig();
	InitPSOs();
}


void VolumeRenderer::Shutdown()
{
	m_rootSig.Destroy();
}


void VolumeRenderer::InitRootSig()
{
	m_rootSig.Reset(2, 3);
	m_rootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Graphics);
	m_rootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 7, ShaderVisibility::Pixel);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerPointClamp(), ShaderVisibility::Pixel);
	m_rootSig.InitStaticSampler(1, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_rootSig.InitStaticSampler(2, CommonStates::SamplerLinearWrap(), ShaderVisibility::Pixel);
	m_rootSig.Finalize("Volume Renderer Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void VolumeRenderer::InitPSOs()
{
	VertexStreamDesc vertexStream{ 0, sizeof(RayVertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(RayVertex, position), InputClassification::PerVertexData, 0 }
	};

	// Comp ray data back
	{
		m_compRayDataBackPSO.SetRootSignature(m_rootSig);
		m_compRayDataBackPSO.SetRenderTargetFormat(Format::R32G32B32A32_Float, Format::Unknown);
		m_compRayDataBackPSO.SetBlendState(CommonStates::BlendDisable());
		m_compRayDataBackPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_compRayDataBackPSO.SetRasterizerState(CommonStates::RasterizerDefaultCW());
		m_compRayDataBackPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_compRayDataBackPSO.SetInputLayout(vertexStream, vertexElements);
		m_compRayDataBackPSO.SetVertexShader("RayDataBackVS");
		m_compRayDataBackPSO.SetPixelShader("RayDataBackPS");
		m_compRayDataBackPSO.Finalize();
	}

	// Comp ray data front
	{
		BlendStateDesc blendDesc{};
		for (auto& renderTargetBlend : blendDesc.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = true;
			renderTargetBlend.srcBlend = Blend::One;
			renderTargetBlend.dstBlend = Blend::Zero;
			renderTargetBlend.blendOp = BlendOp::RevSubtract;
			renderTargetBlend.srcBlendAlpha = Blend::One;
			renderTargetBlend.dstBlendAlpha = Blend::One;
			renderTargetBlend.blendOpAlpha = BlendOp::RevSubtract;
			renderTargetBlend.writeMask = ColorWrite::All;
		}

		m_compRayDataFrontPSO.SetRootSignature(m_rootSig);
		m_compRayDataFrontPSO.SetRenderTargetFormat(Format::R32G32B32A32_Float, Format::Unknown);
		m_compRayDataFrontPSO.SetBlendState(blendDesc);
		m_compRayDataFrontPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_compRayDataFrontPSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_compRayDataFrontPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_compRayDataFrontPSO.SetInputLayout(vertexStream, vertexElements);
		m_compRayDataFrontPSO.SetVertexShader("RayDataFrontVS");
		m_compRayDataFrontPSO.SetPixelShader("RayDataFrontPS");
		m_compRayDataFrontPSO.Finalize();
	}

	// Quad downsample
	{
		m_quadDownsamplePSO.SetRootSignature(m_rootSig);
		m_quadDownsamplePSO.SetRenderTargetFormat(Format::R32G32B32A32_Float, Format::Unknown);
		m_quadDownsamplePSO.SetBlendState(CommonStates::BlendDisable());
		m_quadDownsamplePSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_quadDownsamplePSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_quadDownsamplePSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_quadDownsamplePSO.SetInputLayout(vertexStream, vertexElements);
		m_quadDownsamplePSO.SetVertexShader("RayCastQuadVS");
		m_quadDownsamplePSO.SetPixelShader("RayDataCopyQuadPS");
		m_quadDownsamplePSO.Finalize();
	}

	// Quad raycast
	{
		m_quadRaycastPSO.SetRootSignature(m_rootSig);
		m_quadRaycastPSO.SetRenderTargetFormat(Format::R32G32B32A32_Float, Format::Unknown);
		m_quadRaycastPSO.SetBlendState(CommonStates::BlendDisable());
		m_quadRaycastPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_quadRaycastPSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_quadRaycastPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_quadRaycastPSO.SetInputLayout(vertexStream, vertexElements);
		m_quadRaycastPSO.SetVertexShader("RayCastQuadVS");
		m_quadRaycastPSO.SetPixelShader("RayCastQuadPS");
		m_quadRaycastPSO.Finalize();
	}

	// Quad edge detect
	{
		m_quadEdgeDetectPSO.SetRootSignature(m_rootSig);
		m_quadEdgeDetectPSO.SetRenderTargetFormat(Format::R32G32B32A32_Float, Format::Unknown);
		m_quadEdgeDetectPSO.SetBlendState(CommonStates::BlendDisable());
		m_quadEdgeDetectPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_quadEdgeDetectPSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_quadEdgeDetectPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_quadEdgeDetectPSO.SetInputLayout(vertexStream, vertexElements);
		m_quadEdgeDetectPSO.SetVertexShader("EdgeDetectVS");
		m_quadEdgeDetectPSO.SetPixelShader("EdgeDetectPS");
		m_quadEdgeDetectPSO.Finalize();
	}

	// Quad raycast copy
	{
		BlendStateDesc blendDesc{};
		for (auto& renderTargetBlend : blendDesc.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = true;
			renderTargetBlend.srcBlend = Blend::SrcAlpha;
			renderTargetBlend.dstBlend = Blend::InvSrcAlpha;
			renderTargetBlend.blendOp = BlendOp::Add;
			renderTargetBlend.srcBlendAlpha = Blend::SrcAlpha;
			renderTargetBlend.dstBlendAlpha = Blend::InvSrcAlpha;
			renderTargetBlend.blendOpAlpha = BlendOp::Add;
			renderTargetBlend.writeMask = ColorWrite::All;
		}

		m_quadRaycastCopyPSO.SetRootSignature(m_rootSig);
		m_quadRaycastCopyPSO.SetRenderTargetFormat(Format::R32G32B32A32_Float, Format::Unknown);
		m_quadRaycastCopyPSO.SetBlendState(blendDesc);
		m_quadRaycastCopyPSO.SetDepthStencilState(CommonStates::DepthStateDisabled());
		m_quadRaycastCopyPSO.SetRasterizerState(CommonStates::RasterizerDefault());
		m_quadRaycastCopyPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
		m_quadRaycastCopyPSO.SetInputLayout(vertexStream, vertexElements);
		m_quadRaycastCopyPSO.SetVertexShader("RayCastQuadVS");
		m_quadRaycastCopyPSO.SetPixelShader("RayCastCopyQuadPS");
		m_quadRaycastCopyPSO.Finalize();
	}
}