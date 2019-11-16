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

#include "GpuBuffer.h"
#include "PipelineState.h"
#include "RootSignature.h"
#include "Texture.h"


namespace Kodiak
{

class UIOverlay
{
public:
	void Startup(uint32_t width, uint32_t height);
	void Shutdown();

protected:
	void InitImGui();
	void InitRootSig();
	void InitPSO();
	void InitFontTex();

protected:
	struct Vertex
	{
		float position[2];
		float uv[2];
		float color[4];
	};

	IndexBuffer		m_indexBuffer;
	VertexBuffer	m_vertexBuffer;
	uint32_t		m_indexCount{ 0 };
	uint32_t		m_vertexCount{ 0 };

	RootSignature	m_rootSig;
	GraphicsPSO		m_pso;

	TexturePtr		m_fontTex;

	float			m_scale{ 1.0f };
	uint32_t		m_width{ 0 };
	uint32_t		m_height{ 0 };
};

} // namespace Kodiak