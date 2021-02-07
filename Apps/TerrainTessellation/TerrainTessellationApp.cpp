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

#include "TerrainTessellationApp.h"

#include "Filesystem.h"
#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"
#include "Graphics\GraphicsFeatures.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


class HeightMap : public Texture
{
public:
	void Load(const string& filename, Format format = Format::Unknown, bool sRgb = false)
	{
		auto& filesystem = Filesystem::GetInstance();
		string fullpath = filesystem.GetFullPath(filename);

		assert_msg(!fullpath.empty(), "Could not find texture file %s", filename.c_str());

		string extension = filesystem.GetFileExtension(filename);

		m_retainData = true;

		if (extension == ".dds")
		{	
			LoadDDS(fullpath, format, sRgb);
		}
		else if (extension == ".ktx")
		{
			LoadKTX(fullpath, format, sRgb);
		}
		else
		{
			LoadTexture(fullpath, format, sRgb);
		}
	}
};


void TerrainTessellationApp::Configure()
{
	Application::Configure();

	// Specify required graphics features 
	g_requiredFeatures.tessellationShader = true;
}


void TerrainTessellationApp::Startup()
{
	m_camera.SetPerspectiveMatrix(
		XMConvertToRadians(60.0f),
		(float)m_displayHeight / (float)m_displayWidth,
		0.1f,
		512.0f);
	m_camera.SetPosition(Vector3(18.0f, 22.5f, 57.5f));
	//m_camera.SetRotation(Quaternion(XMConvertToRadians(7.5f), XMConvertToRadians(-343.0f), 0.0f));
	m_camera.Update();

	m_timerSpeed = 0.125f;

	m_controller.SetSpeedScale(0.01f);
	m_controller.SetCameraMode(CameraMode::ArcBall);
	m_controller.SetOrbitTarget(Vector3(0.0f, -2.0f, 0.0f), Length(m_camera.GetPosition()), 4.0f);

	InitRootSigs();
	InitPSOs();
	InitConstantBuffers();

	LoadAssets();

	InitTerrain();

	InitResourceSets();
}


void TerrainTessellationApp::Shutdown()
{
	m_skyRootSig.Destroy();
	m_terrainRootSig.Destroy();
}


bool TerrainTessellationApp::Update()
{
	m_controller.Update(m_frameTimer, m_mouseMoveHandled);

	UpdateConstantBuffers();

	return true;
}


void TerrainTessellationApp::UpdateUI()
{
	// TODO
}


void TerrainTessellationApp::Render()
{
	auto& context = GraphicsContext::Begin("Scene");

	context.TransitionResource(GetColorBuffer(), ResourceState::RenderTarget);
	context.TransitionResource(GetDepthBuffer(), ResourceState::DepthWrite);
	context.ClearColor(GetColorBuffer());
	context.ClearDepth(GetDepthBuffer());

	context.BeginRenderPass(GetBackBuffer());

	context.SetViewportAndScissor(0u, 0u, m_displayWidth, m_displayHeight);

	// Sky sphere
	context.SetRootSignature(m_skyRootSig);
	context.SetPipelineState(m_skyPSO);

	context.SetResources(m_skyResources);

	context.SetIndexBuffer(m_skyModel->GetIndexBuffer());
	context.SetVertexBuffer(0, m_skyModel->GetVertexBuffer());

	context.DrawIndexed((uint32_t)m_skyModel->GetIndexBuffer().GetElementCount());

	// Terrain
	context.SetRootSignature(m_terrainRootSig);
	context.SetPipelineState(m_terrainPSO);

	context.SetResources(m_terrainResources);

	context.SetIndexBuffer(m_terrainIndices);
	context.SetVertexBuffer(0, m_terrainVertices);

	context.DrawIndexed((uint32_t)m_terrainIndices.GetElementCount());

	RenderUI(context);

	context.EndRenderPass();
	context.TransitionResource(GetColorBuffer(), ResourceState::Present);

	context.Finish();
}


void TerrainTessellationApp::InitRootSigs()
{
	m_skyRootSig.Reset(2, 1);
	m_skyRootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_skyRootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);
	m_skyRootSig.InitStaticSampler(0, CommonStates::SamplerLinearWrap(), ShaderVisibility::Pixel);
	m_skyRootSig.Finalize("Sky Sphere Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);

	m_terrainRootSig.Reset(3, 2);
	m_terrainRootSig[0].InitAsDescriptorTable(2, ShaderVisibility::Hull);
	m_terrainRootSig[0].SetTableRange(0, DescriptorType::CBV, 0, 1);
	m_terrainRootSig[0].SetTableRange(1, DescriptorType::TextureSRV, 0, 1);
	m_terrainRootSig[1].InitAsDescriptorTable(2, ShaderVisibility::Domain);
	m_terrainRootSig[1].SetTableRange(0, DescriptorType::CBV, 0, 1);
	m_terrainRootSig[1].SetTableRange(1, DescriptorType::TextureSRV, 0, 1);
	m_terrainRootSig[2].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 2, ShaderVisibility::Pixel);
	m_terrainRootSig.InitStaticSampler(0, CommonStates::SamplerLinearMirror(), ShaderVisibility::All);
	m_terrainRootSig.InitStaticSampler(1, CommonStates::SamplerLinearWrap(), ShaderVisibility::All);
	m_terrainRootSig.Finalize("Terrain Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void TerrainTessellationApp::InitPSOs()
{
	m_skyPSO.SetRootSignature(m_skyRootSig);
	m_skyPSO.SetDepthStencilState(CommonStates::DepthStateReadOnlyReversed());
	m_skyPSO.SetBlendState(CommonStates::BlendDisable());
	m_skyPSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_skyPSO.SetPrimitiveTopology(PrimitiveTopology::TriangleList);
	m_skyPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_skyPSO.SetVertexShader("SkySphereVS");
	m_skyPSO.SetPixelShader("SkySpherePS");

	VertexStreamDesc vertexStream = { 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "NORMAL", 0, Format::R32G32B32_Float, 0, offsetof(Vertex, normal), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 }
		
	};
	m_skyPSO.SetInputLayout(vertexStream, vertexElements);
	m_skyPSO.Finalize();

	m_terrainPSO.SetRootSignature(m_terrainRootSig);
	m_terrainPSO.SetDepthStencilState(CommonStates::DepthStateReadWriteReversed());
	m_terrainPSO.SetBlendState(CommonStates::BlendDisable());
	m_terrainPSO.SetRasterizerState(CommonStates::RasterizerDefault());
	m_terrainPSO.SetPrimitiveTopology(PrimitiveTopology::Patch_4_ControlPoint);
	m_terrainPSO.SetRenderTargetFormat(GetColorFormat(), GetDepthFormat());
	m_terrainPSO.SetVertexShader("TerrainVS");
	m_terrainPSO.SetHullShader("TerrainHS");
	m_terrainPSO.SetDomainShader("TerrainDS");
	m_terrainPSO.SetPixelShader("TerrainPS");
	m_terrainPSO.SetInputLayout(vertexStream, vertexElements);
	m_terrainPSO.Finalize();
}


void TerrainTessellationApp::InitConstantBuffers()
{
	m_skyConstantBuffer.Create("Sky Constant Buffer", 1, sizeof(SkyConstants));
	m_terrainConstantBuffer.Create("Terrain Constant Buffer", 1, sizeof(TerrainConstants));
}


void TerrainTessellationApp::InitResourceSets()
{
	m_skyResources.Init(&m_skyRootSig);
	m_skyResources.SetCBV(0, 0, m_skyConstantBuffer);
	m_skyResources.SetSRV(1, 0, *m_skyTexture);
	m_skyResources.Finalize();

	m_terrainResources.Init(&m_terrainRootSig);
	m_terrainResources.SetCBV(0, 0, m_terrainConstantBuffer);
	m_terrainResources.SetSRV(0, 1, *m_terrainHeightMap);
	m_terrainResources.SetCBV(1, 0, m_terrainConstantBuffer);
	m_terrainResources.SetSRV(1, 1, *m_terrainHeightMap);
	m_terrainResources.SetSRV(2, 0, *m_terrainHeightMap);
	m_terrainResources.SetSRV(2, 1, *m_terrainTextureArray);
	m_terrainResources.Finalize();
}


static float GetTerrainHeight(const uint16_t* data, int x, int y, int dim, int scale)
{
	int rx = x * scale;
	int ry = y * scale;
	rx = max(0, min(rx, dim - 1));
	ry = max(0, min(ry, dim - 1));
	rx /= scale;
	ry /= scale;
	return *(data + (rx + ry * dim) * scale) / 65535.0f;
}


void TerrainTessellationApp::InitTerrain()
{
	const uint32_t PATCH_SIZE = 64;
	const float UV_SCALE = 1.0f;

	HeightMap heightMap;
	heightMap.Load("terrain_heightmap_r16.ktx");

	const uint32_t scale = heightMap.GetWidth() / PATCH_SIZE;

	const uint32_t vertexCount = PATCH_SIZE * PATCH_SIZE;
	unique_ptr<Vertex[]> vertices(new Vertex[vertexCount]);

	const float wx = 2.0f;
	const float wy = 2.0f;

	for (uint32_t x = 0; x < PATCH_SIZE; ++x)
	{
		for (uint32_t y = 0; y < PATCH_SIZE; ++y)
		{
			uint32_t index = x + y * PATCH_SIZE;
			vertices[index].position[0] = x * wx + wx / 2.0f - (float)PATCH_SIZE * wx / 2.0f;
			vertices[index].position[1] = 0.0f;
			vertices[index].position[2] = y * wy + wy / 2.0f - (float)PATCH_SIZE * wy / 2.0f;
			vertices[index].uv[0] = ((float)x / PATCH_SIZE) * UV_SCALE;
			vertices[index].uv[1] = ((float)y / PATCH_SIZE) * UV_SCALE;
		}
	}

	// Calculate normals from height map using a Sobel filter
	const uint16_t* data = (const uint16_t*)heightMap.GetData();
	const uint32_t dim = m_terrainHeightMap->GetWidth();

	for (int x = 0; x < (int)PATCH_SIZE; ++x)
	{
		for (int y = 0; y < (int)PATCH_SIZE; ++y)
		{
			uint32_t index = x + y * PATCH_SIZE;

			// Get height samples centered around current position
			float heights[3][3];
			for (int hx = -1; hx <= 1; hx++)
			{
				for (int hy = -1; hy <= 1; hy++)
				{
					heights[hx + 1][hy + 1] = GetTerrainHeight(data, x + hx, y + hy, dim, scale);
				}
			}

			// Calculate the normal
			Vector3 normal;
			// Gx Sobel filter
			normal.SetX(heights[0][0] - heights[2][0] + 2.0f * heights[0][1] - 2.0f * heights[2][1] + heights[0][2] - heights[2][2]);
			// Gy Sobel filter
			normal.SetZ(heights[0][0] + 2.0f * heights[1][0] + heights[2][0] - heights[0][2] - 2.0f * heights[1][2] - heights[2][2]);
			// Calculate missing up component of the normal using the filtered x and y axis
			// The first value controls the bump strength
			normal.SetY(0.25f * sqrt(1.0f - normal.GetX() * normal.GetX() - normal.GetZ() * normal.GetZ()));

			normal = Normalize(normal * Vector3(2.0f, 1.0f, 2.0f));

			vertices[index].normal[0] = normal.GetX();
			vertices[index].normal[1] = normal.GetY();
			vertices[index].normal[2] = normal.GetZ();
		}
	}

	// Indices
	const uint32_t w = (PATCH_SIZE - 1);
	const uint32_t indexCount = w * w * 4;
	unique_ptr<uint32_t[]> indices(new uint32_t[indexCount]);
	for (uint32_t x = 0; x < w; ++x)
	{
		for (uint32_t y = 0; y < w; ++y)
		{
			uint32_t index = (x + y * w) * 4;
			indices[index] = (x + y * PATCH_SIZE);
			indices[index + 1] = indices[index] + PATCH_SIZE;
			indices[index + 2] = indices[index + 1] + 1;
			indices[index + 3] = indices[index] + 1;
		}
	}

	m_terrainVertices.Create("Terrain Vertices", vertexCount, sizeof(Vertex), false, vertices.get());
	m_terrainIndices.Create("Terrain Indices", indexCount, sizeof(uint32_t), false, indices.get());
}


void TerrainTessellationApp::LoadAssets()
{
	auto layout = VertexLayout(
		{
			VertexComponent::Position,
			VertexComponent::Normal,
			VertexComponent::UV
		});
	m_skyModel = Model::Load("geosphere.obj", layout);
	m_skyTexture = Texture::Load("skysphere_bc3_unorm.ktx");
	m_terrainTextureArray = Texture::Load("terrain_texturearray_bc3_unorm.ktx");
	m_terrainHeightMap = Texture::Load("terrain_heightmap_r16.ktx", Format::R16_UNorm);
}


void TerrainTessellationApp::UpdateConstantBuffers()
{
	Matrix4 viewMatrix = m_camera.GetViewMatrix();
	viewMatrix.SetW(Vector4(0.0f, 0.0f, 0.0f, 1.0f));
	m_skyConstants.modelViewProjectionMatrix = m_camera.GetProjMatrix() * viewMatrix;
	m_skyConstantBuffer.Update(sizeof(SkyConstants), &m_skyConstants);

	m_terrainConstants.modelViewMatrix = m_camera.GetViewMatrix();
	m_terrainConstants.projectionMatrix = m_camera.GetProjMatrix();
	m_terrainConstants.lightPos = Vector4(-48.0f, -40.0f, 46.0f, 0.0f);
	m_terrainConstants.displacementFactor = 32.0f;
	m_terrainConstants.tessellationFactor = 0.75f;
	m_terrainConstants.tessellatedEdgeSize = 20.0f;
	m_terrainConstants.viewportDim[0] = (float)GetWidth();
	m_terrainConstants.viewportDim[1] = (float)GetHeight();

	Frustum frustum = m_camera.GetWorldSpaceFrustum();
	m_terrainConstants.frustumPlanes[0] = frustum.GetFrustumPlane(Frustum::kBottomPlane);
	m_terrainConstants.frustumPlanes[1] = frustum.GetFrustumPlane(Frustum::kTopPlane);
	m_terrainConstants.frustumPlanes[2] = frustum.GetFrustumPlane(Frustum::kLeftPlane);
	m_terrainConstants.frustumPlanes[3] = frustum.GetFrustumPlane(Frustum::kRightPlane);
	m_terrainConstants.frustumPlanes[4] = frustum.GetFrustumPlane(Frustum::kNearPlane);
	m_terrainConstants.frustumPlanes[5] = frustum.GetFrustumPlane(Frustum::kFarPlane);

	m_terrainConstantBuffer.Update(sizeof(TerrainConstants), &m_terrainConstants);
}