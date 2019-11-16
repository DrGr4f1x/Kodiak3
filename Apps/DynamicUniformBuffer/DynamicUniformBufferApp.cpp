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

#include "DynamicUniformBufferApp.h"

#include "CommandContext.h"
#include "CommonStates.h"


using namespace Kodiak;
using namespace std;


void DynamicUniformBufferApp::Startup()
{
	using namespace Math;

	m_camera.SetPerspectiveMatrix(
		DirectX::XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		256.0f);
	m_camera.SetPosition(Vector3(0.0f, 0.0f, -30.0f));
	m_camera.Update();

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, 0.0f, 0.0f), Length(m_camera.GetPosition()), 0.25f);

	InitRootSig();
	InitPSO();
	InitConstantBuffers();
	InitBox();
}


void DynamicUniformBufferApp::Shutdown()
{
	_aligned_free(m_vsModelConstants.modelMatrix);
}


bool DynamicUniformBufferApp::Update()
{
	m_controller.Update(m_frameTimer);

	UpdateConstantBuffers();

	return true;
}


void DynamicUniformBufferApp::Render()
{
	auto& context = GraphicsContext::Begin("Render Frame");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepthAndStencil(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);
	context.SetRootSignature(m_rootSignature);
	context.SetPipelineState(m_pso);
	
	context.SetIndexBuffer(m_indexBuffer);
	context.SetVertexBuffer(0, m_vertexBuffer);

	context.SetConstantArray(0, 32, &m_vsConstants);

	for (uint32_t i = 0; i < m_numCubes; ++i)
	{
		uint32_t dynamicOffset = i * (uint32_t)m_dynamicAlignment;
		
		context.SetConstantArray(0, 16, (byte*)(m_vsModelConstants.modelMatrix) + dynamicOffset, 32);

		context.DrawIndexed((uint32_t)m_indexBuffer.GetElementCount());
	}

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void DynamicUniformBufferApp::InitRootSig()
{
	m_rootSignature.Reset(1);
	m_rootSignature[0].InitAsConstants(0, 48, ShaderVisibility::Vertex);
	m_rootSignature.Finalize("Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout | RootSignatureFlags::DenyPixelShaderRootAccess);
}


void DynamicUniformBufferApp::InitPSO()
{
	m_pso.SetRootSignature(m_rootSignature);
	m_pso.SetBlendState(CommonStates::BlendDisable());
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
	m_pso.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_pso.SetVertexShader("BaseVS");
	m_pso.SetPixelShader("BasePS");
	m_pso.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

	// Vertex inputs
	VertexStreamDesc vertexStream{ 0, 6 * sizeof(float), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, 0, InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32_Float, 0, 3 * sizeof(float), InputClassification::PerVertexData, 0 }
	};

	m_pso.SetInputLayout(vertexStream, vertexElements);
	m_pso.Finalize();
}


void DynamicUniformBufferApp::InitConstantBuffers()
{
	using namespace Math;

	m_dynamicAlignment = AlignUp(sizeof(Matrix4), Limits::ConstantBufferAlignment);

	size_t allocSize = m_numCubes * m_dynamicAlignment;
	m_vsModelConstants.modelMatrix = (Matrix4*)_aligned_malloc(allocSize, m_dynamicAlignment);

	m_vsConstantBuffer.Create("VS Constant Buffer", 1, sizeof(VSConstants));

	UpdateConstantBuffers();

	RandomNumberGenerator rng;
	for (uint32_t i = 0; i < m_numCubes; ++i)
	{
		m_rotations[i] = DirectX::XM_2PI * Vector3(rng.NextFloat(-1.0f, 1.0f), rng.NextFloat(-1.0f, 1.0f), rng.NextFloat(-1.0f, 1.0f));
		m_rotationSpeeds[i] = Vector3(rng.NextFloat(-1.0f, 1.0f), rng.NextFloat(-1.0f, 1.0f), rng.NextFloat(-1.0f, 1.0f));
	}
}


void DynamicUniformBufferApp::InitBox()
{
	struct Vertex
	{
		float pos[3];
		float color[3];
	};

	vector<Vertex> vertices = 
	{
		{ { -1.0f, -1.0f,  1.0f },{ 1.0f, 0.0f, 0.0f } },
		{ {  1.0f, -1.0f,  1.0f },{ 0.0f, 1.0f, 0.0f } },
		{ {  1.0f,  1.0f,  1.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { -1.0f,  1.0f,  1.0f },{ 0.0f, 0.0f, 0.0f } },
		{ { -1.0f, -1.0f, -1.0f },{ 1.0f, 0.0f, 0.0f } },
		{ {  1.0f, -1.0f, -1.0f },{ 0.0f, 1.0f, 0.0f } },
		{ {  1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f, 1.0f } },
		{ { -1.0f,  1.0f, -1.0f },{ 0.0f, 0.0f, 0.0f } },
	};

	m_vertexBuffer.Create("Vertex Buffer", vertices.size(), sizeof(Vertex), vertices.data());

	vector<uint16_t> indices = 
	{
		0,1,2, 2,3,0, // Face 0
		1,5,6, 6,2,1, // ...
		7,6,5, 5,4,7, // ...
		4,0,3, 3,7,4, // ...
		4,5,1, 1,0,4, // ...
		3,2,6, 6,7,3, // Face 5
	};

	m_indexBuffer.Create("Index Buffer", indices.size(), sizeof(uint16_t), indices.data());
}


void DynamicUniformBufferApp::UpdateConstantBuffers()
{
	using namespace Math;

	m_vsConstants.projectionMatrix = m_camera.GetProjMatrix();
	m_vsConstants.viewMatrix = m_camera.GetViewMatrix();

	m_vsConstantBuffer.Update(sizeof(m_vsConstants), &m_vsConstants);

	m_animationTimer += m_frameTimer;
	if (m_animationTimer <= 1.0f / 60.0f)
		return;

	Vector3 offset(5.0f);
	const float dim = static_cast<float>(m_numCubesSide);

	for (uint32_t x = 0; x < m_numCubesSide; ++x)
	{
		for (uint32_t y = 0; y < m_numCubesSide; ++y)
		{
			for (uint32_t z = 0; z < m_numCubesSide; ++z)
			{
				const uint32_t index = x * m_numCubesSide * m_numCubesSide + y * m_numCubesSide + z;

				auto modelMatrix = (Matrix4*)((uint64_t)m_vsModelConstants.modelMatrix + (index * m_dynamicAlignment));

				m_rotations[index] += m_animationTimer * m_rotationSpeeds[index];

				Vector3 pos = {
					-((dim * offset.GetX()) / 2.0f) + offset.GetX() / 2.0f + (float)x * offset.GetX(),
					-((dim * offset.GetY()) / 2.0f) + offset.GetY() / 2.0f + (float)y * offset.GetY(),
					-((dim * offset.GetZ()) / 2.0f) + offset.GetZ() / 2.0f + (float)z * offset.GetZ() };

				Quaternion rotX{ Vector3(1.0f, 1.0f, 0.0f), m_rotations[index].GetX() };
				Quaternion rotY{ Vector3(kYUnitVector), m_rotations[index].GetY() };
				Quaternion rotZ{ Vector3(kZUnitVector), m_rotations[index].GetZ() };
				Quaternion rotCombined = rotZ * rotY * rotX;

				*modelMatrix = AffineTransform{ rotCombined, pos };
			}
		}
	}

	m_animationTimer = 0.0f;
}