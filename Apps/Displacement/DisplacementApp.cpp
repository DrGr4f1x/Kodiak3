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

#include "DisplacementApp.h"

#include "CommonStates.h"
#include "Filesystem.h"


using namespace Kodiak;
using namespace std;


void DisplacementApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Models");
	filesystem.AddSearchPath("Data\\Textures");
}


void DisplacementApp::Startup()
{
	using namespace Math;

	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		512.0f);
	m_camera.SetPosition(Vector3(-0.08f, 3.6f, 8.4f));
	m_camera.Update();

	m_controller.SetSpeedScale(0.025f);

	InitRootSig();
	InitPSOs();
	//InitConstantBuffer();

	//LoadAssets();
}


void DisplacementApp::Shutdown()
{
	m_rootSig.Destroy();
}


void DisplacementApp::InitRootSig()
{
	m_rootSig.Reset(3, 1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Hull);
	m_rootSig[1].InitAsDescriptorTable(2, ShaderVisibility::Domain);
	m_rootSig[1].SetTableRange(0, DescriptorType::CBV, 0, 1);
	m_rootSig[1].SetTableRange(1, DescriptorType::TextureSRV, 0, 1);
	m_rootSig[2].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp());
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void DisplacementApp::InitPSOs()
{
	m_pso.SetRootSignature(m_rootSig);

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_pso.SetVertexShader("BaseVS");
	m_pso.SetPixelShader("BasePS");
	m_pso.SetHullShader("DisplacementHS");
	m_pso.SetDomainShader("DisplacementDS");

	m_pso.SetRenderPass(m_renderPass);

	m_pso.SetPrimitiveTopology(PrimitiveTopology::Patch_3_ControlPoint);

	// Vertex inputs
	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
	};
	m_pso.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);

	m_pso.Finalize();
}