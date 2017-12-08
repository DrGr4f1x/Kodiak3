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
//

#pragma once

#include "InputLayout.h"

namespace Kodiak
{

// Forward declarations
class RenderPass;
class RootSignature;


struct RenderTargetBlendDesc
{
	RenderTargetBlendDesc();
	RenderTargetBlendDesc(Blend srcBlend, Blend dstBlend);

	bool		blendEnable;
	bool		logicOpEnable;
	Blend		srcBlend;
	Blend		dstBlend;
	BlendOp		blendOp;
	Blend		srcBlendAlpha;
	Blend		dstBlendAlpha;
	BlendOp		blendOpAlpha;
	LogicOp		logicOp;
	ColorWrite	writeMask;
};


struct BlendStateDesc
{
	BlendStateDesc();
	BlendStateDesc(Blend srcBlend, Blend dstBlend);

	bool					alphaToCoverageEnable;
	bool					independentBlendEnable;
	RenderTargetBlendDesc	renderTargetBlend[8];
};


struct RasterizerStateDesc
{
	RasterizerStateDesc();
	RasterizerStateDesc(CullMode cullMode, FillMode fillMode);

	CullMode	cullMode;
	FillMode	fillMode;
	bool		frontCounterClockwise;
	int32_t		depthBias;
	float		depthBiasClamp;
	float		slopeScaledDepthBias;
	bool		depthClipEnable;
	bool		multisampleEnable;
	bool		antialiasedLineEnable;
	uint32_t	forcedSampleCount;
	bool		conservativeRasterizationEnable;
};


struct StencilOpDesc
{
	StencilOpDesc();

	StencilOp		stencilFailOp;
	StencilOp		stencilDepthFailOp;
	StencilOp		stencilPassOp;
	ComparisonFunc	stencilFunc;
};


struct DepthStencilStateDesc
{
	DepthStencilStateDesc();
	DepthStencilStateDesc(bool enable, bool writeEnable);

	bool			depthEnable;
	DepthWrite		depthWriteMask;
	ComparisonFunc	depthFunc;
	bool			stencilEnable;
	uint8_t			stencilReadMask;
	uint8_t			stencilWriteMask;
	StencilOpDesc	frontFace;
	StencilOpDesc	backFace;
};


class PSO
{
public:
	static void DestroyAll();

	void SetRootSignature(const RootSignature& bindMappings)
	{
		m_rootSignature = &bindMappings;
	}

	const RootSignature& GetRootSignature() const
	{
		assert(m_rootSignature != nullptr);
		return *m_rootSignature;
	}

	ID3D12PipelineState* GetPipelineStateObject() const { return m_pso; }

protected:
	const RootSignature* m_rootSignature{ nullptr };
	ID3D12PipelineState* m_pso{ nullptr };
};


class GraphicsPSO : public PSO
{
	friend class CommandContext;

public:
	// Start with empty state
	GraphicsPSO();

	void SetBlendState(const BlendStateDesc& blendDesc);
	void SetRasterizerState(const RasterizerStateDesc& rasterizerDesc);
	void SetDepthStencilState(const DepthStencilStateDesc& depthStencilDesc);
	void SetSampleMask(uint32_t sampleMask);
	void SetPrimitiveTopology(PrimitiveTopology topology);
	void SetRenderPass(const RenderPass& renderpass);
	void SetInputLayout(uint32_t numStreams, const VertexStreamDesc* vertexStreams, uint32_t numElements, const VertexElementDesc* inputElementDescs);
	void SetPrimitiveRestart(IndexBufferStripCutValue ibProps);

	void SetVertexShader(const std::string& filename);
	void SetPixelShader(const std::string& filename);
	void SetGeometryShader(const std::string& filename);
	void SetHullShader(const std::string& filename);
	void SetDomainShader(const std::string& filename);

	// Perform validation and compute a hash value for fast state block comparisons
	void Finalize();

	D3D12_PRIMITIVE_TOPOLOGY GetTopology() const { return m_topology; }

private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC m_psoDesc;
	D3D12_PRIMITIVE_TOPOLOGY m_topology;
	std::shared_ptr<const D3D12_INPUT_ELEMENT_DESC> m_inputLayouts;
	std::shared_ptr<const VertexElementDesc> m_inputElements;
};


class ComputePSO : public PSO
{
	friend class CommandContext;

public:
	ComputePSO();

	void SetComputeShader(const std::string& filename);

	void Finalize();

private:
	D3D12_COMPUTE_PIPELINE_STATE_DESC m_psoDesc;
};

} // namespace Kodiak