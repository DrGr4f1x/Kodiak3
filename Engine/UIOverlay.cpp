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

#include "CommonStates.h"
#include "Filesystem.h"
#include "SamplerState.h"

#include "imgui.h"


using namespace Kodiak;
using namespace Math;
using namespace std;


void UIOverlay::Startup(uint32_t width, uint32_t height)
{
	m_width = width;
	m_height = height;

	InitImGui();
	InitRootSig();
	InitPSO();
	InitFontTex();
}


void UIOverlay::Shutdown()
{
	m_rootSig.Destroy();
	m_fontTex.reset();
}


void UIOverlay::Update()
{
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
		m_vertexBuffer.Create("UI Vertex Buffer", imDrawData->TotalVtxCount, sizeof(ImDrawVert));
		m_vertexCount = imDrawData->TotalVtxCount;
	}

	if (m_indexBuffer.GetSize() == 0 || (m_indexCount != imDrawData->TotalIdxCount))
	{
		m_indexBuffer.Create("UI Index Buffer", imDrawData->TotalIdxCount, sizeof(ImDrawIdx));
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
{}


void UIOverlay::InitImGui()
{
	// Init ImGui
	ImGui::CreateContext();

	// Color scheme
	ImGuiStyle& style = ImGui::GetStyle();
	style.Colors[ImGuiCol_TitleBg] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgActive] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	style.Colors[ImGuiCol_TitleBgCollapsed] = ImVec4(1.0f, 0.0f, 0.0f, 0.1f);
	style.Colors[ImGuiCol_MenuBarBg] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_Header] = ImVec4(0.8f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_HeaderHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_FrameBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_CheckMark] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_SliderGrab] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_SliderGrabActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);
	style.Colors[ImGuiCol_FrameBgHovered] = ImVec4(1.0f, 1.0f, 1.0f, 0.1f);
	style.Colors[ImGuiCol_FrameBgActive] = ImVec4(1.0f, 1.0f, 1.0f, 0.2f);
	style.Colors[ImGuiCol_Button] = ImVec4(1.0f, 0.0f, 0.0f, 0.4f);
	style.Colors[ImGuiCol_ButtonHovered] = ImVec4(1.0f, 0.0f, 0.0f, 0.6f);
	style.Colors[ImGuiCol_ButtonActive] = ImVec4(1.0f, 0.0f, 0.0f, 0.8f);

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

	m_pso.SetRenderTargetFormat(Format::R8G8B8A8_UNorm, Format::Unknown);

	// Vertex inputs
	VertexStreamDesc vertexStream = { 0, sizeof(Vertex), InputClassification::PerVertexData };
	vector<VertexElementDesc> vertexElements =
	{
		{ "POSITION", 0, Format::R32G32_Float, 0, offsetof(Vertex, position), InputClassification::PerVertexData, 0 },
		{ "TEXCOORD", 0, Format::R32G32_Float, 0, offsetof(Vertex, uv), InputClassification::PerVertexData, 0 },
		{ "COLOR", 0, Format::R32G32B32A32_Float, 0, offsetof(Vertex, color), InputClassification::PerVertexData, 0 }
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