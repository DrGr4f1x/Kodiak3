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

#include "RadialBlurApp.h"

#include "CommonStates.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"
#include "Shader.h"
#include "SwapChain.h"


using namespace Kodiak;
using namespace std;


void RadialBlurApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	auto binDir = filesystem.GetBinaryDir();
	string rootDir = binDir;
	auto pos = binDir.find("Bin");
	if (pos != binDir.npos)
	{
		rootDir = binDir.substr(0, pos);
	}
	filesystem.SetRootDir(rootDir);

#if defined(DX12)
	filesystem.AddSearchPath("Data\\Shaders\\DXIL");
#elif defined(VK)
	filesystem.AddSearchPath("Data\\Shaders\\SPIR-V");
#else
#error No graphics API defined!
#endif
}


void RadialBlurApp::Startup()
{
	InitRootSigs();
	InitPSOs();
}


void RadialBlurApp::Shutdown()
{
	m_renderPass.Destroy();
	m_offscreenRenderPass.Destroy();

	m_radialBlurRootSig.Destroy();
	m_sceneRootSig.Destroy();
}


void RadialBlurApp::Render()
{}


void RadialBlurApp::InitRenderPasses()
{
	auto swapChain = m_graphicsDevice->GetSwapChain();

	auto colorFormat = swapChain->GetColorFormat();
	auto depthFormat = m_graphicsDevice->GetDepthFormat();
	m_renderPass.AddColorAttachment(colorFormat, ResourceState::Undefined, ResourceState::Present);
	m_renderPass.AddDepthAttachment(depthFormat, ResourceState::Undefined, ResourceState::DepthWrite);
	m_renderPass.Finalize();

	colorFormat = Format::R8G8B8A8_UNorm;
	m_offscreenRenderPass.AddColorAttachment(colorFormat, ResourceState::Undefined, ResourceState::PixelShaderResource);
	m_offscreenRenderPass.AddDepthAttachment(depthFormat, ResourceState::Undefined, ResourceState::DepthWrite);
	m_offscreenRenderPass.Finalize();
}


void RadialBlurApp::InitRootSigs()
{
	m_sceneRootSig.Reset(2, 1);
	m_sceneRootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
	m_sceneRootSig[1].InitAsDescriptorTable(2, ShaderVisibility::Pixel);
	m_sceneRootSig[1].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_sceneRootSig[1].SetTableRange(1, DescriptorType::CBV, 0, 1);
	m_sceneRootSig.InitStaticSampler(0, CommonStates::SamplerLinearWrap(), ShaderVisibility::Pixel);
	m_sceneRootSig.Finalize("Scene Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_radialBlurRootSig.Reset(1, 1);
	m_radialBlurRootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Pixel);
	m_radialBlurRootSig[0].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_radialBlurRootSig[0].SetTableRange(1, DescriptorType::CBV, 0, 1);
	m_radialBlurRootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_radialBlurRootSig.Finalize("Radial Blur Root Sig", RootSignatureFlags::None);
}


void RadialBlurApp::InitPSOs()
{
	m_radialBlurPSO.SetRootSignature(m_radialBlurRootSig);
	m_radialBlurPSO.SetRenderPass(m_renderPass);
	m_radialBlurPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_radialBlurPSO.SetBlendState(CommonStates::BlendTraditionalAdditive());
	m_radialBlurPSO.SetDepthStencilState(CommonStates::DepthStateReadWrite());
	m_radialBlurPSO.SetRasterizerState(CommonStates::RasterizerDefaultCW());
	m_radialBlurPSO.SetVertexShader(Shader::Load("RadialBlurVS"));
	m_radialBlurPSO.SetPixelShader(Shader::Load("RadialBlurPS"));

	m_offscreenDisplayPSO = m_radialBlurPSO;
	m_offscreenDisplayPSO.SetBlendState(CommonStates::BlendDisable());

	m_phongPassPSO = m_offscreenDisplayPSO;
	m_phongPassPSO.SetRootSignature(m_sceneRootSig);
	m_phongPassPSO.SetVertexShader(Shader::Load("PhongPassVS"));
	m_phongPassPSO.SetPixelShader(Shader::Load("PhongPassPS"));

	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "Position", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "TexCoord", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
		{ "Color", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 },
		{ "Normal", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 }
	};
	m_phongPassPSO.SetInputLayout(1, &vertexStreamDesc, 2, vertexElements);

	m_colorPassPSO = m_phongPassPSO;
	m_colorPassPSO.SetRenderPass(m_offscreenRenderPass);
	m_colorPassPSO.SetVertexShader(Shader::Load("ColorPassVS"));
	m_colorPassPSO.SetPixelShader(Shader::Load("ColorPassPS"));

	m_radialBlurPSO.Finalize();
	m_offscreenDisplayPSO.Finalize();
	m_phongPassPSO.Finalize();
	m_colorPassPSO.Finalize();
}