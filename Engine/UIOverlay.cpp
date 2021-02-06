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

#include "UIOverlay.h"

#include "Graphics\CommandContext.h"
#include "Graphics\CommonStates.h"
#include "Filesystem.h"
#include "SamplerState.h"

#include "imgui.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void UIOverlay::Startup(uint32_t width, uint32_t height, Format format, Format depthFormat)
{
	LOG_NOTICE << "  Starting up UI overlay";

	m_width = width;
	m_height = height;
	m_format = format;
	m_depthFormat = depthFormat;

	InitImGui();
	InitRootSig();
	InitPSO();
	InitFontTex();
	InitConstantBuffer();
	InitResourceSet();
}


void UIOverlay::Shutdown()
{
	LOG_NOTICE << "  Shutting down UI overlay";

	m_rootSig.Destroy();
	m_fontTex.reset();
}


void UIOverlay::Update()
{
	UpdateConstantBuffer();

	ImDrawData* imDrawData = ImGui::GetDrawData();

	if (!imDrawData)
		return;

	// Note: Alignment is done inside buffer creation
	uint32_t vertexBufferSize = imDrawData->TotalVtxCount * sizeof(ImDrawVert);
	uint32_t indexBufferSize = imDrawData->TotalIdxCount * sizeof(ImDrawIdx);

	// Update buffers only if vertex or index count has been changed compared to current buffer size
	if ((vertexBufferSize == 0) || (indexBufferSize == 0))
		return;

	if (m_vertexBuffer.GetSize() == 0 || (m_vertexCount != imDrawData->TotalVtxCount))
	{
		m_vertexBuffer.Create("UI Vertex Buffer", imDrawData->TotalVtxCount, sizeof(ImDrawVert), true);
		m_vertexCount = imDrawData->TotalVtxCount;
	}

	if (m_indexBuffer.GetSize() == 0 || (m_indexCount != imDrawData->TotalIdxCount))
	{
		m_indexBuffer.Create("UI Index Buffer", imDrawData->TotalIdxCount, sizeof(ImDrawIdx), true);
		m_indexCount = imDrawData->TotalIdxCount;
	}

	size_t indexOffset = 0;
	size_t vertexOffset = 0;
	for (int n = 0; n < imDrawData->CmdListsCount; n++) 
	{
		const ImDrawList* cmd_list = imDrawData->CmdLists[n];

		m_indexBuffer.Update(cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), indexOffset, cmd_list->IdxBuffer.Data);
		m_vertexBuffer.Update(cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), vertexOffset, cmd_list->VtxBuffer.Data);

		indexOffset += cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx);
		vertexOffset += cmd_list->VtxBuffer.Size * sizeof(ImDrawVert);
	}
}


void UIOverlay::Render(GraphicsContext& context)
{
	ImDrawData* imDrawData = ImGui::GetDrawData();

	if (!imDrawData || imDrawData->CmdListsCount == 0)
		return;

	//ScopedDrawEvent event(context, "UI Overlay");

	context.SetViewportAndScissor(0u, 0u, m_width, m_height);

	context.SetRootSignature(m_rootSig);
	context.SetPipelineState(m_pso);

	context.SetResources(m_resources);

	context.SetVertexBuffer(0, m_vertexBuffer);
	context.SetIndexBuffer(m_indexBuffer);

	ImGuiIO& io = ImGui::GetIO();

	uint32_t indexOffset = 0;
	int32_t vertexOffset = 0;
	for (int i = 0; i < imDrawData->CmdListsCount; ++i)
	{
		const ImDrawList* cmd_list = imDrawData->CmdLists[i];
		for (int j = 0; j < cmd_list->CmdBuffer.Size; ++j)
		{
			const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[j];
			context.SetScissor(
				max((uint32_t)pcmd->ClipRect.x, 0u),
				max((uint32_t)pcmd->ClipRect.y, 0u),
				(uint32_t)pcmd->ClipRect.z,
				(uint32_t)pcmd->ClipRect.w);
			context.DrawIndexed((uint32_t)pcmd->ElemCount, indexOffset, vertexOffset);
			indexOffset += pcmd->ElemCount;
		}
		vertexOffset += cmd_list->VtxBuffer.Size;
	}
}


bool UIOverlay::Header(const char* caption)
{
	return ImGui::CollapsingHeader(caption, ImGuiTreeNodeFlags_DefaultOpen);
}


bool UIOverlay::CheckBox(const char* caption, bool* value)
{
	return ImGui::Checkbox(caption, value);
}


bool UIOverlay::CheckBox(const char* caption, int32_t* value)
{
	bool val = (*value == 1);
	bool res = ImGui::Checkbox(caption, &val);
	*value = val;
	return res;
}


bool UIOverlay::InputFloat(const char* caption, float* value, float step, uint32_t precision)
{
	return ImGui::InputFloat(caption, value, step, step * 10.0f, precision);
}


bool UIOverlay::SliderFloat(const char* caption, float* value, float min, float max)
{
	return ImGui::SliderFloat(caption, value, min, max);
}


bool UIOverlay::SliderInt(const char* caption, int32_t* value, int32_t min, int32_t max)
{
	return ImGui::SliderInt(caption, value, min, max);
}


bool UIOverlay::ComboBox(const char* caption, int32_t* itemindex, vector<string> items)
{
	if (items.empty())
		return false;

	vector<const char*> charitems;
	charitems.reserve(items.size());
	for (size_t i = 0; i < items.size(); ++i) 
	{
		charitems.push_back(items[i].c_str());
	}
	uint32_t itemCount = static_cast<uint32_t>(charitems.size());

	return ImGui::Combo(caption, itemindex, &charitems[0], itemCount, itemCount);
}


bool UIOverlay::Button(const char* caption)
{
	return ImGui::Button(caption);
}


void UIOverlay::Text(const char *formatstr, ...)
{
	va_list args;
	va_start(args, formatstr);
	ImGui::TextV(formatstr, args);
	va_end(args);
}


static inline ImVec4 ColorToImVec4(const Color& color, float alpha)
{
	return ImVec4(color.R(), color.G(), color.B(), alpha);
}


void UIOverlay::InitImGui()
{
	// Init ImGui
	ImGui::CreateContext();

	// Color scheme
	ImGuiStyle& style = ImGui::GetStyle();

#if DX12
	auto colorBright = Color(0.055f, 0.478f, 0.051f);
	auto colorDark = Color(0.044f, 0.382f, 0.0408f);
#else
	auto colorBright = Color(1.0f, 0.0f, 0.0f);
	auto colorDark = Color(0.8f, 0.0f, 0.0f);
#endif

	style.Colors[ImGuiCol_TitleBg] = ColorToImVec4(colorBright, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ColorToImVec4(colorBright, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ColorToImVec4(colorBright, 0.1f);
	style.Colors[ImGuiCol_MenuBarBg] = ColorToImVec4(colorBright, 0.4f);
	style.Colors[ImGuiCol_Header] = ColorToImVec4(colorDark, 0.4f);
	style.Colors[ImGuiCol_HeaderActive] = ColorToImVec4(colorBright, 0.4f);
	style.Colors[ImGuiCol_HeaderHovered] = ColorToImVec4(colorBright, 0.4f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_CheckMark] = ColorToImVec4(colorBright, 0.8f);
	style.Colors[ImGuiCol_SliderGrab] = ColorToImVec4(colorBright, 0.4f);
	style.Colors[ImGuiCol_SliderGrabActive] = ColorToImVec4(colorBright, 0.8f);
	style.Colors[ImGuiCol_FrameBgHovered] = ColorToImVec4(colorBright, 0.1f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
	style.Colors[ImGuiCol_Button] = ColorToImVec4(colorBright, 0.4f);
	style.Colors[ImGuiCol_ButtonHovered] = ColorToImVec4(colorBright, 0.6f);
	style.Colors[ImGuiCol_ButtonActive] = ColorToImVec4(colorBright, 0.8f);

	// Dimensions
	ImGuiIO& io = ImGui::GetIO();
	io.FontGlobalScale = m_scale;
}


void UIOverlay::InitRootSig()
{
	m_rootSig.Reset(2, 1);
	m_rootSig[0].InitAsDescriptorRange(DescriptorType::CBV, 0, 1, ShaderVisibility::Vertex);
	m_rootSig[1].InitAsDescriptorRange(DescriptorType::TextureSRV, 0, 1, ShaderVisibility::Pixel);

	SamplerStateDesc desc{ TextureFilter::MinMagMipLinear, TextureAddress::Border };
	desc.SetBorderColor(Color(1.0f, 1.0f, 1.0f, 1.0f));
	m_rootSig.InitStaticSampler(0, desc, ShaderVisibility::Pixel);
	m_rootSig.Finalize("UI Root Sig", RootSignatureFlags::AllowInputAssemblerInputLayout);
}


void UIOverlay::InitPSO()
{
	m_pso.SetRootSignature(m_rootSig);

	// Custom blending
	{
		BlendStateDesc desc{};
		desc.alphaToCoverageEnable = false;
		desc.independentBlendEnable = false;
		desc.renderTargetBlend[0].blendEnable = true;
		desc.renderTargetBlend[0].srcBlend = Blend::SrcAlpha;
		desc.renderTargetBlend[0].dstBlend = Blend::InvSrcAlpha;
		desc.renderTargetBlend[0].blendOp = BlendOp::Add;
		desc.renderTargetBlend[0].srcBlendAlpha = Blend::InvSrcAlpha;
		desc.renderTargetBlend[0].dstBlendAlpha = Blend::Zero;
		desc.renderTargetBlend[0].blendOpAlpha = BlendOp::Add;
		desc.renderTargetBlend[0].writeMask = ColorWrite::All;

		m_pso.SetBlendState(desc);
	}

	m_pso.SetDepthStencilState(CommonStates::DepthStateDisabled());
	m_pso.SetRasterizerState(CommonStates::RasterizerTwoSided());
	m_pso.SetPrimitiveTopology(PrimitiveTopology::TriangleList);

	m_pso.SetRenderTargetFormat(m_format, m_depthFormat);

	// Vertex inputs
	VertexStreamDesc vertexStream = { 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R8G8B8A8_UNorm, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 }
	};
	m_pso.SetInputLayout(vertexStream, vertexElements);

	m_pso.SetVertexShader("UIVS");
	m_pso.SetPixelShader("UIPS");

	m_pso.Finalize();
}


void UIOverlay::InitFontTex()
{
	ImGuiIO& io = ImGui::GetIO();

	auto& filesystem = Filesystem::GetInstance();

	string fullPath = filesystem.GetFullPath("Roboto-Medium.ttf");
	io.Fonts->AddFontFromFileTTF(fullPath.c_str(), 16.0f);

	unsigned char* fontData{ nullptr };
	int texWidth{ 0 };
	int texHeight{ 0 };

	io.Fonts->GetTexDataAsRGBA32(&fontData, &texWidth, &texHeight);

	m_fontTex = make_shared<Texture>();
	m_fontTex->Create2D(texWidth, texHeight, Format::R8G8B8A8_UNorm, fontData);
}


void UIOverlay::InitConstantBuffer()
{
	m_vsConstantBuffer.Create("UI Constant Buffer", 1, sizeof(VSConstants));
}


void UIOverlay::InitResourceSet()
{
	m_resources.Init(&m_rootSig);
	m_resources.SetCBV(0, 0, m_vsConstantBuffer);
	m_resources.SetSRV(1, 0, *m_fontTex);
	m_resources.Finalize();
}


void UIOverlay::UpdateConstantBuffer()
{
	const ImDrawData* imDrawData = ImGui::GetDrawData();

	const float L = imDrawData->DisplayPos.x;
	const float R = L + imDrawData->DisplaySize.x;
	const float T = imDrawData->DisplayPos.y;
	const float B = T + imDrawData->DisplaySize.y;

	m_vsConstants.projectionMatrix.SetX(Vector4(2.0f / (R - L), 0.0f, 0.0f, 0.0f));
	m_vsConstants.projectionMatrix.SetY(Vector4(0.0f, 2.0f / (T - B), 0.0f, 0.0f));
	m_vsConstants.projectionMatrix.SetZ(Vector4(0.0f, 0.0f, 0.5f, 0.0f));
	m_vsConstants.projectionMatrix.SetW(Vector4((R + L) / (L - R), (T + B) / (B - T), 0.5f, 1.0f));

	m_vsConstantBuffer.Update(sizeof(VSConstants), &m_vsConstants);
}