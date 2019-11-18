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
#include "ResourceSet.h"
#include "RootSignature.h"
#include "Texture.h"


namespace Kodiak
{

// Forward declarations
class GraphicsContext;


class UIOverlay
{
public:
	void Startup(uint32_t width, uint32_t height, Format format);
	void Shutdown();

	void Update();
	void Render(GraphicsContext& context);

	float GetScale() const { return m_scale; }

	bool Header(const char* caption);
	bool CheckBox(const char* caption, bool* value);
	bool CheckBox(const char* caption, int32_t* value);
	bool InputFloat(const char* caption, float* value, float step, uint32_t precision);
	bool SliderFloat(const char* caption, float* value, float min, float max);
	bool SliderInt(const char* caption, int32_t* value, int32_t min, int32_t max);
	bool ComboBox(const char* caption, int32_t* itemindex, std::vector<std::string> items);
	bool Button(const char* caption);
	void Text(const char* formatstr, ...);

protected:
	void InitImGui();
	void InitRootSig();
	void InitPSO();
	void InitFontTex();
	void InitConstantBuffer();
	void InitResourceSet();

	void UpdateConstantBuffer();

protected:
	struct Vertex
	{
		float position[2];
		float uv[2];
		float color[4];
	};

	struct VSConstants
	{
		float scale[2];
		float translate[2];
	};

	IndexBuffer		m_indexBuffer;
	VertexBuffer	m_vertexBuffer;
	uint32_t		m_indexCount{ 0 };
	uint32_t		m_vertexCount{ 0 };

	RootSignature	m_rootSig;
	GraphicsPSO		m_pso;

	TexturePtr		m_fontTex;

	VSConstants		m_vsConstants;
	ConstantBuffer	m_vsConstantBuffer;

	ResourceSet		m_resources;

	float			m_scale{ 1.0f };
	uint32_t		m_width{ 0 };
	uint32_t		m_height{ 0 };
	Format			m_format;
};

} // namespace Kodiak