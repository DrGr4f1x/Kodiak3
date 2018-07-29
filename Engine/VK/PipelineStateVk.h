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


struct ShaderBytecode
{
	const byte* binary;
	size_t size;

	operator bool() const
	{
		return binary != nullptr && size > 0;
	}
};


class PSO
{
public:
	PSO() = default;

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

	VkPipeline GetPipelineStateObject() const { return m_pipeline; }

protected:
	const RootSignature* m_rootSignature{ nullptr };
	VkPipeline m_pipeline{ VK_NULL_HANDLE };
};


class GraphicsPSO : public PSO
{
	friend class CommandContext;

public:
	// Start with empty state
	GraphicsPSO();

	void SetBlendState(const BlendStateDesc& stateDesc);
	void SetRasterizerState(const RasterizerStateDesc& stateDesc);
	void SetDepthStencilState(const DepthStencilStateDesc& stateDesc);
	void SetSampleMask(uint32_t sampleMask);
	void SetMsaaState(uint32_t numSamples);
	void SetPrimitiveTopology(PrimitiveTopology topology);
	void SetRenderPass(RenderPass& renderpass);
	void SetInputLayout(uint32_t numStreams, const VertexStreamDesc* vertexStreams, uint32_t numElements, const VertexElementDesc* inputElementDescs);
	void SetPrimitiveRestart(IndexBufferStripCutValue ibProps);

	void SetVertexShader(const std::string& filename);
	void SetPixelShader(const std::string& filename);
	void SetGeometryShader(const std::string& filename);
	void SetHullShader(const std::string& filename);
	void SetDomainShader(const std::string& filename);

	void Finalize();

private:
	VkRenderPass m_renderPass{ VK_NULL_HANDLE };
	bool m_finalized{ false };

	// Main pipeline description
	VkGraphicsPipelineCreateInfo m_pipelineCreateInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };

	// Render state descriptions
	VkPipelineViewportStateCreateInfo m_viewportStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	VkPipelineColorBlendStateCreateInfo m_blendStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	std::array<VkPipelineColorBlendAttachmentState, 8> m_blendAttachments;
	VkPipelineMultisampleStateCreateInfo m_multisampleInfo{ VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	VkPipelineRasterizationStateCreateInfo m_rasterizationInfo{ VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	VkPipelineDepthStencilStateCreateInfo m_depthStencilInfo{ VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	VkPipelineVertexInputStateCreateInfo m_vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	VkPipelineInputAssemblyStateCreateInfo m_inputAssemblyInfo{ VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	VkPipelineTessellationStateCreateInfo m_tessellationInfo{ VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };

	// Vertex inputs
	std::vector<VkVertexInputBindingDescription> m_vertexInputBindings;
	std::vector<VkVertexInputAttributeDescription> m_vertexAttributes;

	// Shaders
	ShaderBytecode m_vertexShader{ nullptr, 0 };
	ShaderBytecode m_pixelShader{ nullptr, 0 };
	ShaderBytecode m_hullShader{ nullptr, 0 };
	ShaderBytecode m_domainShader{ nullptr, 0 };
	ShaderBytecode m_geometryShader{ nullptr, 0 };

	// Dynamic states
	std::vector<VkDynamicState> m_dynamicStates;
};


class ComputePSO : public PSO
{
	friend class CommandContext;

public:
	ComputePSO();

	void SetComputeShader(const std::string& filename);

	void Finalize();

private:
	ShaderBytecode m_computeShader{ nullptr, 0 };
};

} // namespace Kodiak