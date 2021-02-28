//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

#include "Graphics\GpuBuffer.h"
#include "Graphics\PipelineState.h"
#include "Graphics\ResourceSet.h"
#include "Graphics\RootSignature.h"


// Forward declarations
namespace Math { class Camera; }


namespace Kodiak
{

// Forward declarations
class GraphicsContext;


class Grid
{
public:
	void Startup();
	void Shutdown();

	void Update(const Math::Camera& camera);
	void Render(GraphicsContext& context);

private:
	void InitMesh();
	void InitRootSig();
	void InitPSO();
	void InitResourceSet();

private:
	int m_width{ 10 };
	int m_height{ 10 };
	float m_spacing{ 1.0f };

	// Vertex buffer and attributes
	Kodiak::VertexBuffer m_vertexBuffer;

	// Index buffer
	Kodiak::IndexBuffer m_indexBuffer;

	// Uniform buffer block object
	Kodiak::ConstantBuffer m_constantBuffer;

	struct
	{
		Math::Matrix4 viewProjectionMatrix;
	} m_vsConstants;

	Kodiak::RootSignature	m_rootSig;
	Kodiak::GraphicsPSO		m_pso;

	Kodiak::ResourceSet		m_resources;
};

} // namespace Kodiak