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
#include "Filesystem.h"
#include "GraphicsDevice.h"
#include "Shader.h"


using namespace Kodiak;
using namespace std;


void TextureArrayApp::Configure()
{
	// Setup file system
	auto& filesystem = Kodiak::Filesystem::GetInstance();

	filesystem.SetDefaultRootDir();
	filesystem.AddSearchPath("Data\\" + GetDefaultShaderPath());
	filesystem.AddSearchPath("Data\\Textures");
}


void TextureArrayApp::Startup()
{
	// Setup vertices for a single uv-mapped quad made from two triangles
	vector<Vertex> vertexData =
	{
		{ { 2.5f,  2.5f, 0.0f },{ 1.0f, 1.0f } },
		{ { -2.5f,  2.5f, 0.0f },{ 0.0f, 1.0f } },
		{ { -2.5f, -2.5f, 0.0f },{ 0.0f, 0.0f } },
		{ { 2.5f, -2.5f, 0.0f },{ 1.0f, 0.0f } }
	};
	m_vertexBuffer.Create("Vertex Buffer", vertexData.size(), sizeof(Vertex), vertexData.data());

	// Setup indices
	vector<uint32_t> indexData = { 0,1,2, 2,3,0 };
	m_indexBuffer.Create("Index Buffer", indexData.size(), sizeof(uint32_t), indexData.data());

	InitRootSig();
	InitPSO();

	// We have to load the texture first, so we know how many array slices there are
	LoadAssets();
	InitConstantBuffer();
}


void TextureArrayApp::Shutdown()
{
	m_vertexBuffer.Destroy();
	m_indexBuffer.Destroy();
	m_constantBuffer.Destroy();

	m_texture.reset();

	delete[] m_constants.instance;
}


bool TextureArrayApp::Update()
{
	UpdateConstantBuffer();

	return true;
}


void TextureArrayApp::Render()
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

	context.DrawIndexedInstanced((uint32_t)m_indexBuffer.GetElementCount(), m_layerCount, 0, 0, 0);

	context.EndRenderPass();

	context.Finish();
}


void TextureArrayApp::InitRootSig()
{
	m_rootSig.Reset(2, 1);
	m_rootSig[0].InitAsConstantBuffer(0, ShaderVisibility::Vertex);
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
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadOnlyReversed());

	auto vs = Shader::Load("InstancingVS");
	auto ps = Shader::Load("InstancingPS");

	m_pso.SetVertexShader(vs);
	m_pso.SetPixelShader(ps);

	m_pso.SetRenderPass(m_renderPass);

	m_pso.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	// Vertex inputs
	VertexStreamDesc vertexStreamDesc{ 0, sizeof(Vertex), InputClassification::PerVertexData };
	VertexElementDesc vertexElements[] =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 }
	};
	m_pso.SetInputLayout(1, &vertexStreamDesc, _countof(vertexElements), vertexElements);

	m_pso.Finalize();
}


void TextureArrayApp::InitConstantBuffer()
{
	m_constants.instance = new InstanceData[m_layerCount];
	size_t size = 2 * sizeof(Math::Matrix4) + (m_layerCount * sizeof(InstanceData));

	m_constantBuffer.Create("Constant Buffer", 1, size);

	UpdateConstantBuffer();

	using namespace Math;

	// Setup the per-instance data
	float offset = -1.5f;
	float center = (m_layerCount * offset) / 2.0f;
	for (uint32_t i = 0; i < m_layerCount; ++i)
	{
		auto transform = AffineTransform::MakeTranslation(Vector3(0.0f, i * offset - center, 0.0f));
		transform = transform * AffineTransform::MakeXRotation(DirectX::XMConvertToRadians(60.0f));
		m_constants.instance[i].modelMatrix = transform;
		m_constants.instance[i].arrayIndex.SetX(static_cast<float>(i));
	}
	m_constantBuffer.Update(m_layerCount * sizeof(InstanceData), 2 * sizeof(Matrix4), m_constants.instance);
}


void TextureArrayApp::UpdateConstantBuffer()
{
	using namespace Math;
	using namespace DirectX;

	m_constants.projectionMatrix = Matrix4::MakePerspective(
		XMConvertToRadians(60.0f),
		(float)m_displayWidth / (float)m_displayHeight,
		0.001f,
		256.0f);

	auto transform = AffineTransform::MakeTranslation(Vector3(0.0f, -1.0f, m_zoom));
	transform = transform * AffineTransform::MakeXRotation(XMConvertToRadians(m_rotation.GetX()));
	transform = transform * AffineTransform::MakeYRotation(XMConvertToRadians(m_rotation.GetY()));
	transform = transform * AffineTransform::MakeZRotation(XMConvertToRadians(m_rotation.GetZ()));
	m_constants.viewMatrix = transform;

	// Just update the matrices
	m_constantBuffer.Update(2 * sizeof(Matrix4), &m_constants);
}


void TextureArrayApp::LoadAssets()
{
	m_texture = Texture::Load("texturearray_bc3_unorm.ktx");
	m_layerCount = m_texture->GetArraySize();
}