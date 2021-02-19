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


#include "Graphics\InputLayout.h"

#if defined(DX12)
#include "Graphics\DX12\PipelineStateBase12.h"
#elif defined (VK)
#include "Graphics\VK\PipelineStateBaseVk.h"
#else
#error No graphics API defined!
#endif


namespace Kodiak
{

// Forward declarations
class RootSignature;
class Shader;


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


class GraphicsPSO : public PSO
{
	friend class CommandContext;

public:
	GraphicsPSO();

	void SetBlendState(const BlendStateDesc& blendState);
	void SetRasterizerState(const RasterizerStateDesc& rasterizerDesc);
	void SetDepthStencilState(const DepthStencilStateDesc& depthStencilDesc);
	void SetSampleMask(uint32_t sampleMask);
	void SetPrimitiveTopology(PrimitiveTopology topology);
	void SetRenderTargetFormat(Format rtvFormat, Format dsvFormat, uint32_t msaaCount = 1, bool sampleRateShading = false);
	void SetRenderTargetFormats(std::span<Format> rtvFormats, Format dsvFormat, uint32_t msaaCount = 1, bool sampleRateShading = false);
	void SetInputLayout(const VertexStreamDesc& vertexStream, const std::vector<VertexElementDesc>& inputElementDescs);
	void SetInputLayout(const std::vector<VertexStreamDesc>& vertexStreams, const std::vector<VertexElementDesc>& inputElementDescs);
	void SetPrimitiveRestart(IndexBufferStripCutValue ibProps);

	void SetVertexShader(const std::string& filename);
	void SetPixelShader(const std::string& filename);
	void SetGeometryShader(const std::string& filename);
	void SetHullShader(const std::string& filename);
	void SetDomainShader(const std::string& filename);

	PrimitiveTopology GetTopology() const { return m_topology; }

	// Vulkan derivative PSOs
	void SetParent(GraphicsPSO* pso);

	// Perform validation and compute a hash value for fast state block comparisons
	void Finalize();

private:
	BlendStateDesc				m_blendState;
	DepthStencilStateDesc		m_depthStencilState;
	RasterizerStateDesc			m_rasterizerState;

	uint32_t					m_sampleMask{ 0xFFFFFFFF };

	std::array<Format, 8>		m_rtvFormats;
	Format						m_dsvFormat;
	uint32_t					m_numRtvs{ 0 };
	uint32_t					m_msaaCount{ 1 };
	bool						m_sampleRateShading{ false };

	PrimitiveTopology			m_topology{ PrimitiveTopology::TriangleList };
	IndexBufferStripCutValue	m_ibStripCut{ IndexBufferStripCutValue::Disabled };

	Shader*						m_vertexShader{ nullptr };
	Shader*						m_pixelShader{ nullptr };
	Shader*						m_geometryShader{ nullptr };
	Shader*						m_hullShader{ nullptr };
	Shader*						m_domainShader{ nullptr };

	std::vector<VertexStreamDesc>	m_vertexStreams;
	std::vector<VertexElementDesc>	m_vertexElements;

	// For Vulkan derivative PSOs
	GraphicsPSO*				m_parentPSO{ nullptr };
	bool						m_isParent{ false };

#if _DEBUG
	std::string					m_vertexShaderFile;
	std::string					m_pixelShaderFile;
	std::string					m_geometryShaderFile;
	std::string					m_hullShaderFile;
	std::string					m_domainShaderFile;
#endif
};


class ComputePSO : public PSO
{
	friend class CommandContext;

public:
	void SetComputeShader(const std::string& filename);

	void Finalize();

private:
	Shader* m_computeShader{ nullptr };

#if _DEBUG
	std::string				m_computeShaderFile;
#endif
};

} // namespace Kodiak