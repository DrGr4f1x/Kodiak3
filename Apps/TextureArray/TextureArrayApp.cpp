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

#include "TextureArrayApp.h"

#include "CommandContext.h"
#include "CommonStates.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void TextureArrayApp::Startup()
{
	// Setup vertices for a single uv-mapped quad made from two triangles
	vector<Vertex> vertexData =
	{
		{ {  2.5f,  2.5f,  0.0f }, { 1.0f, 1.0f } },
		{ { -2.5f,  2.5f,  0.0f }, { 0.0f, 1.0f } },
		{ { -2.5f, -2.5f,  0.0f }, { 0.0f, 0.0f } },
		{ {  2.5f, -2.5f,  0.0f }, { 1.0f, 0.0f } }
	};
	m_vertexBuffer.Create("Vertex Buffer", vertexData.size(), sizeof(Vertex), vertexData.data());

	// Setup indices
	vector<uint32_t> indexData = { 0,1,2, 2,3,0 };
	m_indexBuffer.Create("Index Buffer", indexData.size(), sizeof(uint32_t), indexData.data());

	// Setup camera
	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Vector3(0.0f, -1.0f, -m_zoom));

	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.RefreshFromCamera();

	InitRootSig();
	InitPSO();

	// We have to load the texture first, so we know how many array slices there are
	LoadAssets();
	InitConstantBuffer();

	InitResourceSet();
}


void TextureArrayApp::Shutdown()
{
	m_rootSig.Destroy();

	delete[] m_constants.instance;
}


bool TextureArrayApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffer();

	return true;
}


void TextureArrayApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_pso);

	context.SetResources(m_resources);

	context.SetVertexBuffer(0, m_vertexBuffer);
	context.SetIndexBuffer(m_indexBuffer);

	context.DrawIndexedInstanced((uint32_t)m_indexBuffer.GetElementCount(), m_layerCount, 0, 0, 0);

	context.EndRenderPass();

	context.Finish();
}


void TextureArrayApp::InitRootSig()
{
	m_rootSig.Reset(2, 1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig[1].InitAsDescriptorTable(1, ShaderVisibility::Pixel);
	m_rootSig[1].SetTableRange(0, DescriptorType::TextureSRV, 0, 1);
	m_rootSig.InitStaticSampler(0, CommonStates::SamplerLinearClamp(), ShaderVisibility::Pixel);
	m_rootSig.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void TextureArrayApp::InitPSO()
{
	m_pso.SetRootSignature(m_rootSig);

	// Render state
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

	m_pso.SetVertexShader("InstancingVS");
	m_pso.SetPixelShader("InstancingPS");

	m_pso.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	m_pso.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	// Vertex inputs
	VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 }
	};
	m_pso.SetInputLayout(vertexStream, vertexElements);

	m_pso.Finalize();
}


void TextureArrayApp::InitConstantBuffer()
{
	m_constants.instance = new InstanceData[m_layerCount];
	size_t size = sizeof(Matrix4) + (m_layerCount * sizeof(InstanceData));

	m_constantBuffer.Create("Constant Buffer", 1, size);

	UpdateConstantBuffer();

	// Setup the per-instance data
	float offset = 1.5f;
	float center = (m_layerCount * offset) / 2.0f;
	for (uint32_t i = 0; i < m_layerCount; ++i)
	{
		auto transform = AffineTransform::MakeTranslation(Vector3(0.0f, i * offset - center, 0.0f));
		transform = transform * AffineTransform::MakeXRotation(DirectX::XMConvertToRadians(-60.0f));
		m_constants.instance[i].modelMatrix = transform;
		m_constants.instance[i].arrayIndex.SetX(static_cast<float>(i));
	}
	m_constantBuffer.Update(m_layerCount * sizeof(InstanceData), sizeof(Matrix4), m_constants.instance);
}


void TextureArrayApp::InitResourceSet()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_constantBuffer);
	m_resources.SetSRV(1, 0, *m_texture);
	m_resources.Finalize();
}


void TextureArrayApp::UpdateConstantBuffer()
{
	m_constants.viewProjectionMatrix = m_camera.GetViewProjMatrix();

	// Just update the vp matrix
	m_constantBuffer.Update(sizeof(Matrix4), &m_constants);
}


void TextureArrayApp::LoadAssets()
{
	m_texture = Texture::Load("texturearray_bc3_unorm.ktx");
	m_layerCount = m_texture->GetArraySize();
}