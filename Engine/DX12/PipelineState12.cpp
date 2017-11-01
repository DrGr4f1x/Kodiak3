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


GraphicsPSO::GraphicsPSO()
{
	ZeroMemory(&m_psoDesc, sizeof(m_psoDesc));
	m_psoDesc.NodeMask = 1;
	m_psoDesc.SampleMask = 0xFFFFFFFFu;
	m_psoDesc.SampleDesc.Count = 1;
	m_psoDesc.InputLayout.NumElements = 0;
}


void GraphicsPSO::SetBlendState(const D3D12_BLEND_DESC& blendDesc)
{
	m_psoDesc.BlendState = blendDesc;
}


void GraphicsPSO::SetRasterizerState(const D3D12_RASTERIZER_DESC& rasterizerDesc)
{
	m_psoDesc.RasterizerState = rasterizerDesc;
}


void GraphicsPSO::SetDepthStencilState(const D3D12_DEPTH_STENCIL_DESC& depthStencilDesc)
{
	m_psoDesc.DepthStencilState = depthStencilDesc;
}


void GraphicsPSO::SetSampleMask(uint32_t sampleMask)
{
	m_psoDesc.SampleMask = sampleMask;
}


void GraphicsPSO::SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType)
{
	assert_msg(topologyType != D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED, "Can't draw with undefined topology");
	m_psoDesc.PrimitiveTopologyType = topologyType;
}


void GraphicsPSO::SetPrimitiveRestart(D3D12_INDEX_BUFFER_STRIP_CUT_VALUE ibProps)
{
	m_psoDesc.IBStripCutValue = ibProps;
}


void GraphicsPSO::SetRenderTargetFormat(Format rtvFormat, Format dsvFormat, uint32_t msaaCount, uint32_t msaaQuality)
{
	SetRenderTargetFormats(1, &rtvFormat, dsvFormat, msaaCount, msaaQuality);
}


void GraphicsPSO::SetRenderTargetFormats(uint32_t numRTVs, const Format* rtvFormats, Format dsvFormat, uint32_t msaaCount, uint32_t msaaQuality)
{
	assert_msg(numRTVs == 0 || rtvFormats != nullptr, "Null format array conflicts with non-zero length");
	for (uint32_t i = 0; i < numRTVs; ++i)
	{
		m_psoDesc.RTVFormats[i] = static_cast<DXGI_FORMAT>(rtvFormats[i]);
	}
	for (uint32_t i = numRTVs; i < m_psoDesc.NumRenderTargets; ++i)
	{
		m_psoDesc.RTVFormats[i] = DXGI_FORMAT_UNKNOWN;
	}
	m_psoDesc.NumRenderTargets = numRTVs;
	m_psoDesc.DSVFormat = static_cast<DXGI_FORMAT>(dsvFormat);
	m_psoDesc.SampleDesc.Count = msaaCount;
	m_psoDesc.SampleDesc.Quality = msaaQuality;
}


void GraphicsPSO::SetInputLayout(uint32_t numElements, const D3D12_INPUT_ELEMENT_DESC* pInputElementDescs)
{
	m_psoDesc.InputLayout.NumElements = numElements;

	if (numElements > 0)
	{
		D3D12_INPUT_ELEMENT_DESC* newElements = (D3D12_INPUT_ELEMENT_DESC*)malloc(sizeof(D3D12_INPUT_ELEMENT_DESC) * numElements);
		memcpy(newElements, pInputElementDescs, numElements * sizeof(D3D12_INPUT_ELEMENT_DESC));
		m_inputLayouts.reset((const D3D12_INPUT_ELEMENT_DESC*)newElements);
	}
	else
	{
		m_inputLayouts = nullptr;
	}
}


void GraphicsPSO::SetVertexShader(const Shader* vertexShader)
{
	m_psoDesc.VS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(vertexShader->GetByteCode()), vertexShader->GetByteCodeSize());
}


void GraphicsPSO::SetPixelShader(const Shader* pixelShader)
{
	m_psoDesc.PS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(pixelShader->GetByteCode()), pixelShader->GetByteCodeSize());
}


void GraphicsPSO::SetGeometryShader(const Shader* geometryShader)
{
	m_psoDesc.GS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(geometryShader->GetByteCode()), geometryShader->GetByteCodeSize());
}


void GraphicsPSO::SetHullShader(const Shader* hullShader)
{
	m_psoDesc.HS = CD3DX12_SHADER_BYTECODE(const_cast<byte*>(hullShader->GetByteCode()), hullShader->GetByteCodeSize());
}


void GraphicsPSO::SetDomainShader(const Shader* domainShader)
{
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


void ComputePSO::SetComputeShader(const Shader* computeShader)
{
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