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

#include "PipelineState.h"

#include "Hash.h"
#include "Shader.h"


using namespace Kodiak;
using namespace std;





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


GraphicsPSO::GraphicsPSO()
{
	for (uint32_t i = 0; i < 8; ++i)
	{
		m_rtvFormats[i] = Format::Unknown;
	}
	m_dsvFormat = Format::Unknown;
}


void GraphicsPSO::SetBlendState(const BlendStateDesc& blendState)
{
	m_blendState = blendState;
}


void GraphicsPSO::SetRasterizerState(const RasterizerStateDesc& rasterizerState)
{
	m_rasterizerState = rasterizerState;
}


void GraphicsPSO::SetDepthStencilState(const DepthStencilStateDesc& depthStencilState)
{
	m_depthStencilState = depthStencilState;
}


void GraphicsPSO::SetSampleMask(uint32_t sampleMask)
{
	m_sampleMask = sampleMask;
}


void GraphicsPSO::SetPrimitiveTopology(PrimitiveTopology topology)
{
	m_topology = topology;
}


void GraphicsPSO::SetRenderTargetFormat(Format rtvFormat, Format dsvFormat, uint32_t msaaCount, uint32_t msaaQuality)
{
	SetRenderTargetFormats(1, &rtvFormat, dsvFormat, msaaCount, msaaQuality);
}


void GraphicsPSO::SetRenderTargetFormats(uint32_t numRtvs, const Format* rtvFormats, Format dsvFormat, uint32_t msaaCount, uint32_t msaaQuality)
{
	for (uint32_t i = 0; i < numRtvs; ++i)
	{
		m_rtvFormats[i] = rtvFormats[i];
	}
	m_numRtvs = numRtvs;
	m_dsvFormat = dsvFormat;
	m_msaaCount = msaaCount;
	m_msaaQuality = msaaQuality;
}


void GraphicsPSO::SetInputLayout(const VertexStreamDesc& vertexStream, const vector<VertexElementDesc>& inputElementDescs)
{
	m_vertexStreams.push_back(vertexStream);
	m_vertexElements = inputElementDescs;
}

void GraphicsPSO::SetInputLayout(const vector<VertexStreamDesc>& vertexStreams, const vector<VertexElementDesc>& inputElementDescs)
{
	m_vertexStreams = vertexStreams;
	m_vertexElements = inputElementDescs;
}


void GraphicsPSO::SetPrimitiveRestart(IndexBufferStripCutValue ibProps)
{
	m_ibStripCut = ibProps;
}


void GraphicsPSO::SetVertexShader(const string& filename)
{
	m_vertexShader = Shader::Load(filename);
}


void GraphicsPSO::SetPixelShader(const string& filename)
{
	m_pixelShader = Shader::Load(filename);
}


void GraphicsPSO::SetGeometryShader(const string& filename)
{
	m_geometryShader = Shader::Load(filename);
}


void GraphicsPSO::SetHullShader(const string& filename)
{
	m_hullShader = Shader::Load(filename);
}


void GraphicsPSO::SetDomainShader(const string& filename)
{
	m_domainShader = Shader::Load(filename);
}


void ComputePSO::SetComputeShader(const string& filename)
{
	m_computeShader = Shader::Load(filename);
}