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

#include "SmokeSimApp.h"

#include "Input.h"
#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"
#include "Graphics\GraphicsFeatures.h"

#include <iostream>


using namespace Kodiak;
using namespace DirectX;
using namespace std;

void SmokeSimApp::Configure()
{
	Application::Configure();

	g_requiredFeatures.geometryShader = true;
	g_requiredFeatures.imagelessFramebuffer = true;
}


void SmokeSimApp::Startup()
{
	using namespace Math;

	InitRootSigs();
	SetupScene();

	vector<BoundingBox> boxes;
	for (const auto& obj : m_sceneObjects)
	{
		boxes.push_back(obj.model->GetBoundingBox());
	}
	BoundingBox sceneBB = BoundingBoxUnion(boxes);
	
	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.001f,
		256.0f);
	m_camera.SetPosition(Math::Vector3(-3.38f, 2.0f, -7.25f));
	m_camera.Focus(sceneBB);

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(sceneBB.GetCenter(), Length(m_camera.GetPosition()), 4.0f);

	// Initialize fluid simulation
	m_fluidEngine.Initialize(m_gridWidth, m_gridHeight, m_gridDepth);
	m_volumeRenderer.Initialize(m_gridWidth, m_gridHeight, m_gridDepth);
	m_voxelizer.Initialize(
		m_fluidEngine.GetRenderTarget(FluidEngine::RenderTarget::Obstacles), 
		m_fluidEngine.GetRenderTarget(FluidEngine::RenderTarget::ObstacleVelocity));

	Matrix4 scale = Matrix4::MakeScale(4.0f);
	Matrix4 trans = Matrix4::MakeTranslation(0.0f, 4.0f, 0.0f);

	Matrix4 viewMatrix = Matrix4(Vector3(kYUnitVector), Vector3(kZUnitVector), Vector3(kXUnitVector), Vector3(kZero));

	m_gridToWorldMatrix = trans * scale;
	m_worldToGridMatrix = Invert(m_gridToWorldMatrix);

	m_voxelizer.SetGridToWorldMatrix(m_gridToWorldMatrix);

	int objIdx = 0;
	for (auto& obj : m_sceneObjects)
	{
		if (objIdx++ == 0)
		{
			continue;
		}
		m_voxelizer.AddModel(obj.model);
	}
}


void SmokeSimApp::Shutdown()
{
	m_meshRootSig.Destroy();
	m_voxelizer.Shutdown();
	m_volumeRenderer.Shutdown();
	m_fluidEngine.Shutdown();
}


bool SmokeSimApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	// Store previous matrices (for velocity calculation)
	for (const auto& obj : m_sceneObjects)
	{
		obj.model->StorePrevMatrix();
	}

	// Animate the sphere
	{
		float x = 3.0f * sinf(m_appElapsedTime);

		using namespace Math;
		m_sceneObjects[3].model->SetMatrix(Matrix4(AffineTransform(Vector3(x, 4.0f, 0.0f))));
	}

	if (g_input.IsFirstPressed(DigitalInput::kKey_add))
	{
		++m_debugTex;
	}
	else if (g_input.IsFirstPressed(DigitalInput::kKey_subtract))
	{
		--m_debugTex;
	}

	m_debugTex = max(0, min(3, m_debugTex));

	UpdateConstantBuffers();

	m_voxelizer.Update(m_frameTimer);
	m_fluidEngine.Update(m_frameTimer, m_debugTex);

	return true;
}


void SmokeSimApp::UpdateUI()
{
	/*if (m_uiOverlay->Header("Settings"))
	{
		m_uiOverlay->CheckBox("Ortho Cam", &m_bUseOrthoCamera);
		m_uiOverlay->SliderInt("Depth Slice", &m_curSlice, 0, int(m_gridDepth - 1));
	}*/
}

/*
* High-level algorithm
* Per-frame
*   Initialize fluid grid if needed
*   Update fluid
*   Voxelize scene
*   If debug mode
*     Draw selected volume texture
*   Else
*     Render 3D scene
*     Render smoke
*   UI, whatever
*/

void SmokeSimApp::Render()
{
	auto& context = GraphicsContext::Begin("Frame");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	m_voxelizer.Render(context);

	if (m_debugTex > 0)
	{
		m_fluidEngine.PrepareDebugDraw(context);
	}


	context.BeginRenderPass(GetBackBuffer());

	if (m_debugTex > 0)
	{
		m_fluidEngine.DebugDraw(context);
	}
	else
	{
		context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

		RenderScene(context);
	}

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	// Grid and UI
	RenderGrid(context);
	RenderUI(context);

	context.EndRenderPass();

	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void SmokeSimApp::InitRootSigs()
{
	m_meshRootSig.Reset(1);
	m_meshRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_meshRootSig.Finalize("Mesh Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void SmokeSimApp::UpdateConstantBuffers()
{
	using namespace Math;
	for (uint32_t i = 0; i < 4; ++i)
	{
		auto& obj = m_sceneObjects[i];

#if 0
		if (m_bUseOrthoCamera)
		{
			float zNear = float(m_curSlice) / float(m_gridDepth) - 0.5f;
			float zFar = 100000.0f;
			//Matrix4 projectionMatrix = Matrix4(XMMatrixOrthographicOffCenterRH(-0.5f, 0.5f, -0.5f, 0.5f, zNear, zFar));
			Matrix4 projectionMatrix = Matrix4(XMMatrixOrthographicRH(2.0f, 2.0f, zNear, zFar));

			/*Vector3 viewPos{ 0.0f, 8.0f, 0.0f };
			Vector3 target{ 0.0f, 0.0f, 0.0f };
			Vector3 up{ 1.0f, 0.0f, 0.0f };
			Matrix4 viewMatrix = Matrix4(XMMatrixLookAtRH(viewPos, target, up));*/
			Matrix4 viewMatrix = Matrix4(Vector3(kYUnitVector), Vector3(kZUnitVector), Vector3(kXUnitVector), Vector3(kZero));

			obj.constants.projectionMatrix = projectionMatrix;
			obj.constants.modelMatrix = viewMatrix * m_worldToGridMatrix * obj.model->GetMatrix();
		}
		else
#endif
		{
			obj.constants.projectionMatrix = m_camera.GetProjMatrix();
			obj.constants.modelMatrix = m_camera.GetViewMatrix() * obj.model->GetMatrix();
		}

		obj.constantBuffer.Update(sizeof(obj.constants), &obj.constants);
	}
}


void SmokeSimApp::SetupScene()
{
	for (int i = 0; i < 4; ++i)
	{
		auto& obj = m_sceneObjects[i];

		// Setup the PSO
		{
			obj.objectPSO.SetRootSignature(m_meshRootSig);
			obj.objectPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());

			obj.objectPSO.SetBlendState(CommonStates::BlendDisable());
			obj.objectPSO.SetRasterizerState(CommonStates::RasterizerDefault());
			obj.objectPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());

			obj.objectPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleStrip);
			obj.objectPSO.SetPrimitiveRestart(IndexBufferStripCutValue::Value_0xFFFF);

			obj.objectPSO.SetVertexShader("MeshVS");
			obj.objectPSO.SetPixelShader("MeshPS");

			VertexStreamDesc vertexStream{ 0, sizeof(Vertex), InputClassification::PerVertexData };
			vector<VertexElementDesc> vertexElements =
			{
				{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
				{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 }

			};
			obj.objectPSO.SetInputLayout(vertexStream, vertexElements);
			obj.objectPSO.Finalize();
		}

		// Create constant buffer
		obj.constantBuffer.Create("Constant Buffer", 1, sizeof(Constants));

		// Initialize resource set
		obj.resources.Init(&m_meshRootSig);
		obj.resources.SetCBV(0, 0, obj.constantBuffer);
		obj.resources.Finalize();
	}

	// Create models
	auto layout = VertexLayout<VertexComponent::PositionNormal>();
	m_sceneObjects[0].model = Model::MakeBox(layout, 8.0f, 0.25f, 8.0f);
	m_sceneObjects[1].model = Model::MakeCylinder(layout, 4.0f, 0.25f, 32);
	m_sceneObjects[2].model = Model::MakeCylinder(layout, 6.0f, 0.25f, 32);
	m_sceneObjects[3].model = Model::MakeSphere(layout, 1.0f, 32, 32);

	// Set initial constants
	using namespace Math;
	m_sceneObjects[0].constants.color = DirectX::Colors::AntiqueWhite;
	m_sceneObjects[1].model->SetMatrix(Matrix4(AffineTransform(Vector3(2.0f, 0.0f, 2.0f))));
	m_sceneObjects[1].constants.color = DirectX::Colors::Red;
	m_sceneObjects[2].model->SetMatrix(Matrix4(AffineTransform(Vector3(-2.0f, 0.0f, -2.0f))));
	m_sceneObjects[2].constants.color = DirectX::Colors::Red;
	m_sceneObjects[3].model->SetMatrix(Matrix4(AffineTransform(Vector3(0.0f, 4.0f, 0.0f))));
	m_sceneObjects[3].constants.color = DirectX::Colors::Blue;

	// Update constant buffers
	UpdateConstantBuffers();
}


void SmokeSimApp::RenderScene(GraphicsContext& context)
{
	ScopedDrawEvent event(context, "3D Scene");

	context.SetRootSignature(m_meshRootSig);
	context.SetPrimitiveTopology(PrimitiveTopology::TriangleStrip);

	for (auto& obj : m_sceneObjects)
	{
		context.SetResources(obj.resources);
		context.SetPipelineState(obj.objectPSO);
		obj.model->Render(context);
	}
}