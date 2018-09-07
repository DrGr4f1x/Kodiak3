//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard

#include "Stdafx.h"

#include "PipelineState.h"

#include "GraphicsDevice.h"
#include "Hash.h"
#include "Shader.h"

#include "RootSignature12.h"


using namespace Kodiak;
using namespace std;


namespace
{
static map<size_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> s_graphicsPSOHashMap;
static map<size_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> s_computePSOHashMap;
} // anonymous namespace


void PSO::DestroyAll()
{
	s_graphicsPSOHashMap.clear();
	s_computePSOHashMap.clear();
}


void GraphicsPSO::Finalize()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc = {};
	desc.NodeMask = 1;
	desc.SampleMask = m_sampleMask;
	desc.SampleDesc.Count = m_msaaCount;
	desc.SampleDesc.Quality = m_msaaQuality;
	desc.InputLayout.NumElements = 0;

	// Blend state
	desc.BlendState.AlphaToCoverageEnable = m_blendState.alphaToCoverageEnable ? TRUE : FALSE;
	desc.BlendState.IndependentBlendEnable = m_blendState.independentBlendEnable ? TRUE : FALSE;

	for (uint32_t i = 0; i < 8; ++i)
	{
		auto& rtDesc = desc.BlendState.RenderTarget[i];

		rtDesc.BlendEnable = m_blendState.renderTargetBlend[i].blendEnable ? TRUE : FALSE;
		rtDesc.LogicOpEnable = m_blendState.renderTargetBlend[i].logicOpEnable ? TRUE : FALSE;
		rtDesc.SrcBlend = static_cast<D3D12_BLEND>(m_blendState.renderTargetBlend[i].srcBlend);
		rtDesc.DestBlend = static_cast<D3D12_BLEND>(m_blendState.renderTargetBlend[i].dstBlend);
		rtDesc.BlendOp = static_cast<D3D12_BLEND_OP>(m_blendState.renderTargetBlend[i].blendOp);
		rtDesc.SrcBlendAlpha = static_cast<D3D12_BLEND>(m_blendState.renderTargetBlend[i].srcBlendAlpha);
		rtDesc.DestBlendAlpha = static_cast<D3D12_BLEND>(m_blendState.renderTargetBlend[i].dstBlendAlpha);
		rtDesc.BlendOpAlpha = static_cast<D3D12_BLEND_OP>(m_blendState.renderTargetBlend[i].blendOpAlpha);
		rtDesc.LogicOp = static_cast<D3D12_LOGIC_OP>(m_blendState.renderTargetBlend[i].logicOp);
		rtDesc.RenderTargetWriteMask = static_cast<UINT8>(m_blendState.renderTargetBlend[i].writeMask);
	}

	// Rasterizer state
	desc.RasterizerState.FillMode = static_cast<D3D12_FILL_MODE>(m_rasterizerState.fillMode);
	desc.RasterizerState.CullMode = static_cast<D3D12_CULL_MODE>(m_rasterizerState.cullMode);
	desc.RasterizerState.FrontCounterClockwise = m_rasterizerState.frontCounterClockwise ? TRUE : FALSE;
	desc.RasterizerState.DepthBias = m_rasterizerState.depthBias;
	desc.RasterizerState.DepthBiasClamp = m_rasterizerState.depthBiasClamp;
	desc.RasterizerState.SlopeScaledDepthBias = m_rasterizerState.slopeScaledDepthBias;
	desc.RasterizerState.DepthClipEnable = m_rasterizerState.depthClipEnable ? TRUE : FALSE;
	desc.RasterizerState.MultisampleEnable = m_rasterizerState.multisampleEnable ? TRUE : FALSE;
	desc.RasterizerState.AntialiasedLineEnable = m_rasterizerState.antialiasedLineEnable ? TRUE : FALSE;
	desc.RasterizerState.ForcedSampleCount = m_rasterizerState.forcedSampleCount;
	desc.RasterizerState.ConservativeRaster =
		static_cast<D3D12_CONSERVATIVE_RASTERIZATION_MODE>(m_rasterizerState.conservativeRasterizationEnable ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);

	// Depth-stencil state
	desc.DepthStencilState.DepthEnable = m_depthStencilState.depthEnable ? TRUE : FALSE;
	desc.DepthStencilState.DepthWriteMask = static_cast<D3D12_DEPTH_WRITE_MASK>(m_depthStencilState.depthWriteMask);
	desc.DepthStencilState.DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(m_depthStencilState.depthFunc);
	desc.DepthStencilState.StencilEnable = m_depthStencilState.stencilEnable ? TRUE : FALSE;
	desc.DepthStencilState.StencilReadMask = m_depthStencilState.stencilReadMask;
	desc.DepthStencilState.StencilWriteMask = m_depthStencilState.stencilWriteMask;
	desc.DepthStencilState.FrontFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(m_depthStencilState.frontFace.stencilFailOp);
	desc.DepthStencilState.FrontFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(m_depthStencilState.frontFace.stencilDepthFailOp);
	desc.DepthStencilState.FrontFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(m_depthStencilState.frontFace.stencilPassOp);
	desc.DepthStencilState.FrontFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(m_depthStencilState.frontFace.stencilFunc);
	desc.DepthStencilState.BackFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(m_depthStencilState.backFace.stencilFailOp);
	desc.DepthStencilState.BackFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(m_depthStencilState.backFace.stencilDepthFailOp);
	desc.DepthStencilState.BackFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(m_depthStencilState.backFace.stencilPassOp);
	desc.DepthStencilState.BackFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(m_depthStencilState.backFace.stencilFunc);

	// Primitive topology & primitive restart
	desc.PrimitiveTopologyType = MapPrimitiveTopologyToD3DType(m_topology);
	desc.IBStripCutValue = static_cast<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE>(m_ibStripCut);

	// Render target formats
	for (uint32_t i = 0; i < m_numRtvs; ++i)
	{
		desc.RTVFormats[i] = static_cast<DXGI_FORMAT>(m_rtvFormats[i]);
	}
	for (uint32_t i = m_numRtvs; i < desc.NumRenderTargets; ++i)
	{
		desc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}
	desc.NumRenderTargets = m_numRtvs;
	desc.DSVFormat = static_cast<DXGI_FORMAT>(m_dsvFormat);
	desc.SampleDesc.Count = m_msaaCount;
	desc.SampleDesc.Quality = m_msaaQuality;

	// Input layout
	desc.InputLayout.NumElements = (UINT)m_vertexElements.size();
	unique_ptr<const D3D12_INPUT_ELEMENT_DESC> d3dElements;

	if (desc.InputLayout.NumElements > 0)
	{
		D3D12_INPUT_ELEMENT_DESC* newD3DElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * desc.InputLayout.NumElements);

		for (uint32_t i = 0; i < desc.InputLayout.NumElements; ++i)
		{
			newD3DElements[i].AlignedByteOffset = m_vertexElements[i].alignedByteOffset;
			newD3DElements[i].Format = static_cast<DXGI_FORMAT>(m_vertexElements[i].format);
			newD3DElements[i].InputSlot = m_vertexElements[i].inputSlot;
			newD3DElements[i].InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>(m_vertexElements[i].inputClassification);
			newD3DElements[i].InstanceDataStepRate = m_vertexElements[i].instanceDataStepRate;
			newD3DElements[i].SemanticIndex = m_vertexElements[i].semanticIndex;
			newD3DElements[i].SemanticName = m_vertexElements[i].semanticName;
		}

		d3dElements.reset((const D3D12_INPUT_ELEMENT_DESC*)newD3DElements);
	}

	// Shaders
	if (m_vertexShader != nullptr)
	{
		desc.VS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(m_vertexShader->GetByteCode()), m_vertexShader->GetByteCodeSize());
	}

	if (m_pixelShader != nullptr)
	{
		desc.PS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(m_pixelShader->GetByteCode()), m_pixelShader->GetByteCodeSize());
	}

	if (m_geometryShader != nullptr)
	{
		desc.GS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(m_geometryShader->GetByteCode()), m_geometryShader->GetByteCodeSize());
	}

	if (m_hullShader != nullptr)
	{
		desc.HS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(m_hullShader->GetByteCode()), m_hullShader->GetByteCodeSize());
	}

	if (m_domainShader != nullptr)
	{
		desc.DS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(m_domainShader->GetByteCode()), m_domainShader->GetByteCodeSize());
	}

	// Make sure the root signature is finalized first
	desc.pRootSignature = m_rootSignature->GetSignature();
	assert(desc.pRootSignature != nullptr);

	desc.InputLayout.pInputElementDescs = nullptr;

	size_t hashCode = Utility::HashState(&desc);
	hashCode = Utility::HashState(m_vertexElements.data(), desc.InputLayout.NumElements, hashCode);

	desc.InputLayout.pInputElementDescs = d3dElements.get();

	ID3D12PipelineState** PSORef = nullptr;
	bool firstCompile = false;
	{
		static mutex s_hashMapMutex;
		lock_guard<mutex> CS(s_hashMapMutex);

		auto iter = s_graphicsPSOHashMap.find(hashCode);

		// Reserve space so the next inquiry will find that someone got here first.
		if (iter == s_graphicsPSOHashMap.end())
		{
			firstCompile = true;
			PSORef = s_graphicsPSOHashMap[hashCode].GetAddressOf();
		}
		else
		{
			PSORef = iter->second.GetAddressOf();
		}
	}

	if (firstCompile)
	{
		assert_succeeded(GetDevice()->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&m_handle)));
		s_graphicsPSOHashMap[hashCode].Attach(m_handle);
	}
	else
	{
		while (*PSORef == nullptr)
		{
			this_thread::yield();
		}
		m_handle = *PSORef;
	}
}


void ComputePSO::Finalize()
{
	assert(m_computeShader);

	D3D12_COMPUTE_PIPELINE_STATE_DESC desc = {};
	desc.NodeMask = 1;

	desc.CS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(m_computeShader->GetByteCode()), m_computeShader->GetByteCodeSize());
	
	// Make sure the root signature is finalized first
	desc.pRootSignature = m_rootSignature->GetSignature();
	assert(desc.pRootSignature != nullptr);

	size_t hashCode = Utility::HashState(&desc);

	ID3D12PipelineState** PSORef = nullptr;
	bool firstCompile = false;
	{
		static mutex s_hashMapMutex;
		lock_guard<mutex> CS(s_hashMapMutex);

		auto iter = s_computePSOHashMap.find(hashCode);

		// Reserve space so the next inquiry will find that someone got here first.
		if (iter == s_computePSOHashMap.end())
		{
			firstCompile = true;
			PSORef = s_computePSOHashMap[hashCode].GetAddressOf();
		}
		else
		{
			PSORef = iter->second.GetAddressOf();
		}
	}

	if (firstCompile)
	{
		assert_succeeded(GetDevice()->CreateComputePipelineState(&desc, IID_PPV_ARGS(&m_handle)));
		s_computePSOHashMap[hashCode].Attach(m_handle);
	}
	else
	{
		while (*PSORef == nullptr)
		{
			this_thread::yield();
		}
		m_handle = *PSORef;
	}
}