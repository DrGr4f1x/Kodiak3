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

#include "PipelineState12.h"

#include "Hash.h"
#include "Shader.h"

#include "GraphicsDevice12.h"
#include "RenderPass12.h"
#include "RootSignature12.h"


using namespace Kodiak;
using namespace std;


namespace
{

static map<size_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> s_graphicsPSOHashMap;
static map<size_t, Microsoft::WRL::ComPtr<ID3D12PipelineState>> s_computePSOHashMap;

} // anonymous namespace


RenderTargetBlendDesc::RenderTargetBlendDesc()
	: blendEnable(false)
	, logicOpEnable(false)
	, srcBlend(Blend::One)
	, dstBlend(Blend::Zero)
	, blendOp(BlendOp::Add)
	, srcBlendAlpha(Blend::One)
	, dstBlendAlpha(Blend::Zero)
	, blendOpAlpha(BlendOp::Add)
	, logicOp(LogicOp::Noop)
	, writeMask(ColorWrite::All)
{}


RenderTargetBlendDesc::RenderTargetBlendDesc(Blend srcBlend, Blend dstBlend)
	: blendEnable((srcBlend != Blend::One) || (dstBlend != Blend::Zero))
	, logicOpEnable(false)
	, srcBlend(srcBlend)
	, dstBlend(dstBlend)
	, blendOp(BlendOp::Add)
	, srcBlendAlpha(srcBlend)
	, dstBlendAlpha(dstBlend)
	, blendOpAlpha(BlendOp::Add)
	, logicOp(LogicOp::Noop)
	, writeMask(ColorWrite::All)
{}


BlendStateDesc::BlendStateDesc()
	: alphaToCoverageEnable(false)
	, independentBlendEnable(false)
{}


BlendStateDesc::BlendStateDesc(Blend srcBlend, Blend dstBlend)
	: alphaToCoverageEnable(false)
	, independentBlendEnable(false)
{
	renderTargetBlend[0] = RenderTargetBlendDesc(srcBlend, dstBlend);
}


RasterizerStateDesc::RasterizerStateDesc()
	: cullMode(CullMode::Back)
	, fillMode(FillMode::Solid)
	, frontCounterClockwise(false)
	, depthBias(0)
	, slopeScaledDepthBias(0.0f)
	, depthBiasClamp(0.0f)
	, depthClipEnable(true)
	, multisampleEnable(false)
	, antialiasedLineEnable(false)
	, forcedSampleCount(0)
	, conservativeRasterizationEnable(false)
{}


RasterizerStateDesc::RasterizerStateDesc(CullMode cullMode, FillMode fillMode)
	: cullMode(cullMode)
	, fillMode(fillMode)
	, frontCounterClockwise(true)
	, depthBias(0)
	, slopeScaledDepthBias(0.0f)
	, depthBiasClamp(0.0f)
	, depthClipEnable(true)
	, multisampleEnable(false)
	, antialiasedLineEnable(false)
	, forcedSampleCount(0)
	, conservativeRasterizationEnable(false)
{}


StencilOpDesc::StencilOpDesc()
	: stencilFailOp(StencilOp::Keep)
	, stencilDepthFailOp(StencilOp::Keep)
	, stencilPassOp(StencilOp::Keep)
	, stencilFunc(ComparisonFunc::Always)
{}


DepthStencilStateDesc::DepthStencilStateDesc()
	: depthEnable(true)
	, depthWriteMask(DepthWrite::All)
	, depthFunc(ComparisonFunc::Less)
	, stencilEnable(false)
	, stencilReadMask(Defaults::StencilReadMask)
	, stencilWriteMask(Defaults::StencilWriteMask)
	, frontFace()
	, backFace()
{}


DepthStencilStateDesc::DepthStencilStateDesc(bool enable, bool writeEnable)
	: depthEnable(enable)
	, depthWriteMask(writeEnable ? DepthWrite::All : DepthWrite::Zero)
	, depthFunc(ComparisonFunc::Less)
	, stencilEnable(false)
	, stencilReadMask(Defaults::StencilReadMask)
	, stencilWriteMask(Defaults::StencilWriteMask)
	, frontFace()
	, backFace()
{}


void PSO::DestroyAll()
{
	s_graphicsPSOHashMap.clear();
	s_computePSOHashMap.clear();
}


GraphicsPSO::GraphicsPSO()
{
	ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
	m_psoDesc.NodeMask = 1;
	m_psoDesc.SampleMask = 0xFFFFFFFFu;
	m_psoDesc.SampleDesc.Count = 1;
	m_psoDesc.InputLayout.NumElements = 0;
}


void GraphicsPSO::SetBlendState(const BlendStateDesc& blendDesc)
{
	m_psoDesc.BlendState.AlphaToCoverageEnable = blendDesc.alphaToCoverageEnable ? TRUE : FALSE;
	m_psoDesc.BlendState.IndependentBlendEnable = blendDesc.independentBlendEnable ? TRUE : FALSE;

	for (uint32_t i = 0; i < 8; ++i)
	{
		auto& rtDesc = m_psoDesc.BlendState.RenderTarget[i];

		rtDesc.BlendEnable = blendDesc.renderTargetBlend[i].blendEnable ? TRUE : FALSE;
		rtDesc.LogicOpEnable = blendDesc.renderTargetBlend[i].logicOpEnable ? TRUE : FALSE;
		rtDesc.SrcBlend = static_cast<D3D12_BLEND>(blendDesc.renderTargetBlend[i].srcBlend);
		rtDesc.DestBlend = static_cast<D3D12_BLEND>(blendDesc.renderTargetBlend[i].dstBlend);
		rtDesc.BlendOp = static_cast<D3D12_BLEND_OP>(blendDesc.renderTargetBlend[i].blendOp);
		rtDesc.SrcBlendAlpha = static_cast<D3D12_BLEND>(blendDesc.renderTargetBlend[i].srcBlendAlpha);
		rtDesc.DestBlendAlpha = static_cast<D3D12_BLEND>(blendDesc.renderTargetBlend[i].dstBlendAlpha);
		rtDesc.BlendOpAlpha = static_cast<D3D12_BLEND_OP>(blendDesc.renderTargetBlend[i].blendOpAlpha);
		rtDesc.LogicOp = static_cast<D3D12_LOGIC_OP>(blendDesc.renderTargetBlend[i].logicOp);
		rtDesc.RenderTargetWriteMask = static_cast<UINT8>(blendDesc.renderTargetBlend[i].writeMask);
	}
}


void GraphicsPSO::SetRasterizerState(const RasterizerStateDesc& rasterizerDesc)
{
	m_psoDesc.RasterizerState.FillMode = static_cast<D3D12_FILL_MODE>(rasterizerDesc.fillMode);
	m_psoDesc.RasterizerState.CullMode = static_cast<D3D12_CULL_MODE>(rasterizerDesc.cullMode);
	m_psoDesc.RasterizerState.FrontCounterClockwise = rasterizerDesc.frontCounterClockwise ? TRUE : FALSE;
	m_psoDesc.RasterizerState.DepthBias = rasterizerDesc.depthBias;
	m_psoDesc.RasterizerState.DepthBiasClamp = rasterizerDesc.depthBiasClamp;
	m_psoDesc.RasterizerState.SlopeScaledDepthBias = rasterizerDesc.slopeScaledDepthBias;
	m_psoDesc.RasterizerState.DepthClipEnable = rasterizerDesc.depthClipEnable ? TRUE : FALSE;
	m_psoDesc.RasterizerState.MultisampleEnable = rasterizerDesc.multisampleEnable ? TRUE : FALSE;
	m_psoDesc.RasterizerState.AntialiasedLineEnable = rasterizerDesc.antialiasedLineEnable ? TRUE : FALSE;
	m_psoDesc.RasterizerState.ForcedSampleCount = rasterizerDesc.forcedSampleCount;
	m_psoDesc.RasterizerState.ConservativeRaster =
		static_cast<D3D12_CONSERVATIVE_RASTERIZATION_MODE>(rasterizerDesc.conservativeRasterizationEnable ? D3D12_CONSERVATIVE_RASTERIZATION_MODE_ON : D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF);
}


void GraphicsPSO::SetDepthStencilState(const DepthStencilStateDesc& depthStencilDesc)
{
	m_psoDesc.DepthStencilState.DepthEnable = depthStencilDesc.depthEnable ? TRUE : FALSE;
	m_psoDesc.DepthStencilState.DepthWriteMask = static_cast<D3D12_DEPTH_WRITE_MASK>(depthStencilDesc.depthWriteMask);
	m_psoDesc.DepthStencilState.DepthFunc = static_cast<D3D12_COMPARISON_FUNC>(depthStencilDesc.depthFunc);
	m_psoDesc.DepthStencilState.StencilEnable = depthStencilDesc.stencilEnable ? TRUE : FALSE;
	m_psoDesc.DepthStencilState.StencilReadMask = depthStencilDesc.stencilReadMask;
	m_psoDesc.DepthStencilState.StencilWriteMask = depthStencilDesc.stencilWriteMask;
	m_psoDesc.DepthStencilState.FrontFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.frontFace.stencilFailOp);
	m_psoDesc.DepthStencilState.FrontFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.frontFace.stencilDepthFailOp);
	m_psoDesc.DepthStencilState.FrontFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.frontFace.stencilPassOp);
	m_psoDesc.DepthStencilState.FrontFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(depthStencilDesc.frontFace.stencilFunc);
	m_psoDesc.DepthStencilState.BackFace.StencilFailOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.backFace.stencilFailOp);
	m_psoDesc.DepthStencilState.BackFace.StencilDepthFailOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.backFace.stencilDepthFailOp);
	m_psoDesc.DepthStencilState.BackFace.StencilPassOp = static_cast<D3D12_STENCIL_OP>(depthStencilDesc.backFace.stencilPassOp);
	m_psoDesc.DepthStencilState.BackFace.StencilFunc = static_cast<D3D12_COMPARISON_FUNC>(depthStencilDesc.backFace.stencilFunc);
}


void GraphicsPSO::SetSampleMask(uint32_t sampleMask)
{
	m_psoDesc.SampleMask = sampleMask;
}


void GraphicsPSO::SetPrimitiveTopology(PrimitiveTopology topology)
{
	m_psoDesc.PrimitiveTopologyType = MapPrimitiveTopologyToD3DType(topology);
	m_topology = static_cast<D3D12_PRIMITIVE_TOPOLOGY>(topology);
}


void GraphicsPSO::SetPrimitiveRestart(IndexBufferStripCutValue ibProps)
{
	m_psoDesc.IBStripCutValue = static_cast<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE>(ibProps);
}


void GraphicsPSO::SetRenderPass(const RenderPass& renderpass)
{
	uint32_t numRTVs = static_cast<uint32_t>(renderpass.GetNumColorAttachments());
	for (uint32_t i = 0; i < numRTVs; ++i)
	{
		m_psoDesc.RTVFormats[i] = static_cast<DXGI_FORMAT>(renderpass.GetColorFormat(i));
	}
	for (uint32_t i = numRTVs; i < m_psoDesc.NumRenderTargets; ++i)
	{
		m_psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}
	m_psoDesc.NumRenderTargets = numRTVs;
	m_psoDesc.DSVFormat = static_cast<DXGI_FORMAT>(renderpass.GetDepthFormat());
	m_psoDesc.SampleDesc.Count = 1;
	m_psoDesc.SampleDesc.Quality = 0;
}


void GraphicsPSO::SetInputLayout(uint32_t numStreams, const VertexStreamDesc* vertexStreams, uint32_t numElements, const VertexElementDesc* inputElementDescs)
{
	m_psoDesc.InputLayout.NumElements = numElements;

	if (numElements > 0)
	{
		VertexElementDesc* newElements = (VertexElementDesc*)malloc(sizeof(VertexElementDesc) * numElements);
		memcpy(newElements, inputElementDescs, numElements * sizeof(VertexElementDesc));
		m_inputElements.reset((const VertexElementDesc*)newElements);

		D3D12_INPUT_ELEMENT_DESC* newD3DElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * numElements);

		for (uint32_t i = 0; i < numElements; ++i)
		{
			newD3DElements[i].AlignedByteOffset = inputElementDescs[i].alignedByteOffset;
			newD3DElements[i].Format = static_cast<DXGI_FORMAT>(inputElementDescs[i].format);
			newD3DElements[i].InputSlot = inputElementDescs[i].inputSlot;
			newD3DElements[i].InputSlotClass = static_cast<D3D12_INPUT_CLASSIFICATION>(inputElementDescs[i].inputClassification);
			newD3DElements[i].InstanceDataStepRate = inputElementDescs[i].instanceDataStepRate;
			newD3DElements[i].SemanticIndex = inputElementDescs[i].semanticIndex;
			newD3DElements[i].SemanticName = newElements[i].semanticName;
		}

		m_inputLayouts.reset((const D3D12_INPUT_ELEMENT_DESC*)newD3DElements);

	}
	else
	{
		m_inputElements = nullptr;
		m_inputLayouts = nullptr;
	}
}


void GraphicsPSO::SetVertexShader(const string& filename)
{
	auto vertexShader = Shader::Load(filename);
	m_psoDesc.VS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(vertexShader->GetByteCode()), vertexShader->GetByteCodeSize());
}


void GraphicsPSO::SetPixelShader(const string& filename)
{
	auto pixelShader = Shader::Load(filename);
	m_psoDesc.PS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(pixelShader->GetByteCode()), pixelShader->GetByteCodeSize());
}


void GraphicsPSO::SetGeometryShader(const string& filename)
{
	auto geometryShader = Shader::Load(filename);
	m_psoDesc.GS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(geometryShader->GetByteCode()), geometryShader->GetByteCodeSize());
}


void GraphicsPSO::SetHullShader(const string& filename)
{
	auto hullShader = Shader::Load(filename);
	m_psoDesc.HS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(hullShader->GetByteCode()), hullShader->GetByteCodeSize());
}


void GraphicsPSO::SetDomainShader(const string& filename)
{
	auto domainShader = Shader::Load(filename);
	m_psoDesc.DS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(domainShader->GetByteCode()), domainShader->GetByteCodeSize());
}


void GraphicsPSO::Finalize()
{
	// Make sure the root signature is finalized first
	m_psoDesc.pRootSignature = m_rootSignature->GetSignature();
	assert(m_psoDesc.pRootSignature != nullptr);

	m_psoDesc.InputLayout.pInputElementDescs = nullptr;

	size_t hashCode = Utility::HashState(&m_psoDesc);
	hashCode = Utility::HashState(m_inputLayouts.get(), m_psoDesc.InputLayout.NumElements, hashCode);

	m_psoDesc.InputLayout.pInputElementDescs = m_inputLayouts.get();

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
		assert_succeeded(GetDevice()->CreateGraphicsPipelineState(&m_psoDesc, MY_IID_PPV_ARGS(&m_pso)));
		s_graphicsPSOHashMap[hashCode].Attach(m_pso);
	}
	else
	{
		while (*PSORef == nullptr)
		{
			this_thread::yield();
		}
		m_pso = *PSORef;
	}
}


ComputePSO::ComputePSO()
{
	ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
	m_psoDesc.NodeMask = 1;
}


void ComputePSO::SetComputeShader(const string& filename)
{
	auto computeShader = Shader::Load(filename);
	m_psoDesc.CS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(computeShader->GetByteCode()), computeShader->GetByteCodeSize());
}


void ComputePSO::Finalize()
{
	// Make sure the root signature is finalized first
	m_psoDesc.pRootSignature = m_rootSignature->GetSignature();
	assert(m_psoDesc.pRootSignature != nullptr);

	size_t hashCode = Utility::HashState(&m_psoDesc);

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
		assert_succeeded(GetDevice()->CreateComputePipelineState(&m_psoDesc, MY_IID_PPV_ARGS(&m_pso)));
		s_computePSOHashMap[hashCode].Attach(m_pso);
	}
	else
	{
		while (*PSORef == nullptr)
		{
			this_thread::yield();
		}
		m_pso = *PSORef;
	}
}