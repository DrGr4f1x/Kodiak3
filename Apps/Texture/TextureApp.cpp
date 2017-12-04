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

#include "TextureApp.h"

#include "CommandContext.h"
#include "CommonStates.h"
#include "Filesystem.h"
#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


void TextureApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Textures");
}


void TextureApp::Startup()
{
	// Setup vertices for a single uv-mapped quad made from two triangles
	vector<Vertex> vertexData =
	{
		{ { 1.0f,  1.0f, 0.0f },{ 1.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { -1.0f,  1.0f, 0.0f },{ 0.0f, 1.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { -1.0f, -1.0f, 0.0f },{ 0.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { 1.0f, -1.0f, 0.0f },{ 1.0f, 0.0f },{ 0.0f, 0.0f, 1.0f } }
	};
	m_vertexBuffer.Create("Vertex buffer", vertexData.size(), sizeof(Vertex), vertexData.data());

	vector<uint32_t> indexData = { 0,1,2, 2,3,0 };
	m_indexBuffer.Create("Index buffer", indexData.size(), sizeof(uint32_t), indexData.data());

	m_camera.SetPerspectiveMatrix(DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(0.0f, 0.0f, -m_zoom));

	InitRootSig();
	InitPSO();
	InitConstantBuffer();

	LoadAssets();
}


void TextureApp::Shutdown()
{
	m_vertexBuffer.Destroy();
	m_indexBuffer.Destroy();
	m_constantBuffer.Destroy();

	m_rootSig.Destroy();

	m_texture.reset();
}


bool TextureApp::Update()
{
	UpdateConstantBuffer();

	return true;
}


void TextureApp::Render()
{
	auto& context = GraphicsContext::Begin("Render frame");

	uint32_t curFrame = m_graphicsDevice->GetCurrentBuffer();

	Color clearColor{ DirectX::Colors::Black };
	context.BeginRenderPass(m_renderPass, *m_framebuffers[curFrame], clearColor, 1.0f, 0);

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_pso);

	context.SetRootConstantBuffer(0, m_constantBuffer);
	context.SetSRV(1, 0, *m_texture);

	context.SetVertexBuffer(0, m_vertexBuffer);
	context.SetIndexBuffer(m_indexBuffer);

	context.DrawIndexed((uint32_t)m_indexBuffer.GetElementCount());

	context.EndRenderPass();

	context.Finish();
}


void TextureApp::InitRootSig()
{
	m_rootSig.Reset(2, 1);
	m_rootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
	m_rootSig[1].InitAsDescriptorTable(1, ShaderVisibility::Pixel);
	m_rootSig[1].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void TextureApp::InitPSO()
{
	m_pso.SetRootSignature(m_rootSig);

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadOnlyReversed());

	m_pso.SetVertexShader("TextureVS");
	m_pso.SetPixelShader("TexturePS");

	m_pso.SetRenderPass(m_renderPass);

	m_pso.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	// Vertex inputs
	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 }
	};
	m_pso.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);

	m_pso.Finalize();
}


void TextureApp::InitConstantBuffer()
{
	m_constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

	UpdateConstantBuffer();
}


void TextureApp::UpdateConstantBuffer()
{
	using namespace Math;

	m_constants.projectionMatrix = m_camera.GetProjMatrix();
	m_constants.modelMatrix = m_camera.GetViewMatrix();
	m_constants.viewPos = m_camera.GetPosition();

	m_constantBuffer.Update(sizeof(Constants), &m_constants);
}


void TextureApp::LoadAssets()
{
	m_texture = Texture::Load("metalplate01_rgba.ktx");
}