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

#include "PipelineStateVk.h"

#include "Hash.h"
#include "Shader.h"

#include "GraphicsDeviceVk.h"
#include "RenderPassVk.h"


using namespace Kodiak;
using namespace std;


namespace
{

mutex s_pipelineMutex;
set<VkPipeline> s_graphicsPipelineCache;
set<VkPipeline> s_computePipelineCache;
VkPipelineCache s_pipelineCache{ VK_NULL_HANDLE };

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
	lock_guard<mutex> CS(s_pipelineMutex);

	auto device = GetDevice();

	for (auto& pso : s_graphicsPipelineCache)
	{
		vkDestroyPipeline(device, pso, nullptr);
	}
	s_graphicsPipelineCache.clear();

	for (auto& pso : s_computePipelineCache)
	{
		vkDestroyPipeline(device, pso, nullptr);
	}
	s_computePipelineCache.clear();

	vkDestroyPipelineCache(device, s_pipelineCache, nullptr);
	s_pipelineCache = VK_NULL_HANDLE;
}


GraphicsPSO::GraphicsPSO()
{
	m_pipelineCreateInfo.pNext = nullptr;
	m_pipelineCreateInfo.flags = 0;

	m_viewportStateInfo.pNext = nullptr;
	m_viewportStateInfo.flags = 0;
	m_viewportStateInfo.viewportCount = 1;
	m_viewportStateInfo.pViewports = nullptr;
	m_viewportStateInfo.scissorCount = 1;
	m_viewportStateInfo.pScissors = nullptr;

	m_blendStateInfo.pNext = nullptr;
	m_blendStateInfo.flags = 0;
	m_blendStateInfo.logicOpEnable = VK_FALSE;
	m_blendStateInfo.logicOp = VK_LOGIC_OP_SET;
	m_blendStateInfo.blendConstants[0] = 1.0f;
	m_blendStateInfo.blendConstants[1] = 1.0f;
	m_blendStateInfo.blendConstants[2] = 1.0f;
	m_blendStateInfo.blendConstants[3] = 1.0f;
	m_blendStateInfo.attachmentCount = 1;
	ZeroMemory(&m_blendAttachments, 8 * sizeof(VkPipelineColorBlendAttachmentState));

	m_multisampleInfo.pNext = nullptr;
	m_multisampleInfo.flags = 0;
	m_multisampleInfo.sampleShadingEnable = VK_FALSE;
	m_multisampleInfo.alphaToOneEnable = VK_FALSE;
	m_multisampleInfo.alphaToCoverageEnable = VK_FALSE;
	uint32_t smplMask = 0x1;
	m_multisampleInfo.pSampleMask = nullptr;
	m_multisampleInfo.minSampleShading = 0.0f;
	m_multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	m_rasterizationInfo.pNext = nullptr;
	m_rasterizationInfo.flags = 0;
	m_rasterizationInfo.lineWidth = 1.0f;
	m_rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	m_rasterizationInfo.depthClampEnable = VK_FALSE;

	m_depthStencilInfo.pNext = nullptr;
	m_depthStencilInfo.flags = 0;
	m_depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	m_depthStencilInfo.minDepthBounds = 0.0f;
	m_depthStencilInfo.maxDepthBounds = 1.0f;

	m_vertexInputInfo.pNext = nullptr;
	m_vertexInputInfo.flags = 0;

	m_inputAssemblyInfo.pNext = nullptr;
	m_inputAssemblyInfo.flags = 0;

	m_dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	m_dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);
}


void GraphicsPSO::SetBlendState(const BlendStateDesc& stateDesc)
{
	for (uint32_t i = 0; i < 8; ++i)
	{
		const auto& rt = stateDesc.renderTargetBlend[i];
		m_blendAttachments[i].blendEnable = rt.blendEnable ? VK_TRUE : VK_FALSE;
		m_blendAttachments[i].srcColorBlendFactor = static_cast<VkBlendFactor>(rt.srcBlend);
		m_blendAttachments[i].dstColorBlendFactor = static_cast<VkBlendFactor>(rt.dstBlend);
		m_blendAttachments[i].colorBlendOp = static_cast<VkBlendOp>(rt.blendOp);
		m_blendAttachments[i].srcAlphaBlendFactor = static_cast<VkBlendFactor>(rt.srcBlendAlpha);
		m_blendAttachments[i].dstAlphaBlendFactor = static_cast<VkBlendFactor>(rt.dstBlendAlpha);
		m_blendAttachments[i].alphaBlendOp = static_cast<VkBlendOp>(rt.blendOpAlpha);
		m_blendAttachments[i].colorWriteMask = static_cast<VkColorComponentFlags>(rt.writeMask);

		// First render target with logic op enabled gets to set the state
		if (rt.logicOpEnable && (VK_FALSE == m_blendStateInfo.logicOpEnable))
		{
			m_blendStateInfo.logicOpEnable = VK_TRUE;
			m_blendStateInfo.logicOp = static_cast<VkLogicOp>(rt.logicOp);
		}
	}
	m_blendStateInfo.attachmentCount = 1;
	m_blendStateInfo.pAttachments = &m_blendAttachments[0];

	m_multisampleInfo.alphaToCoverageEnable = stateDesc.alphaToCoverageEnable ? VK_TRUE : VK_FALSE;
}


void GraphicsPSO::SetRasterizerState(const RasterizerStateDesc& stateDesc)
{
	m_rasterizationInfo.polygonMode = static_cast<VkPolygonMode>(stateDesc.fillMode);
	m_rasterizationInfo.cullMode = static_cast<VkCullModeFlags>(stateDesc.cullMode);
	m_rasterizationInfo.frontFace = stateDesc.frontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
	m_rasterizationInfo.depthBiasEnable = (stateDesc.depthBias != 0 || stateDesc.slopeScaledDepthBias != 0.0f) ? VK_TRUE : VK_FALSE;
	m_rasterizationInfo.depthBiasClamp = stateDesc.depthBiasClamp;
	m_rasterizationInfo.depthBiasConstantFactor = *reinterpret_cast<const float*>(&stateDesc.depthBias);
	m_rasterizationInfo.depthBiasSlopeFactor = stateDesc.slopeScaledDepthBias;
}


void GraphicsPSO::SetDepthStencilState(const DepthStencilStateDesc& stateDesc)
{
	m_depthStencilInfo.depthTestEnable = stateDesc.depthEnable ? VK_TRUE : VK_FALSE;
	m_depthStencilInfo.depthWriteEnable = (stateDesc.depthWriteMask == DepthWrite::All) ? VK_TRUE : VK_FALSE;
	m_depthStencilInfo.depthCompareOp = static_cast<VkCompareOp>(stateDesc.depthFunc);
	m_depthStencilInfo.stencilTestEnable = stateDesc.stencilEnable ? VK_TRUE : VK_FALSE;

	m_depthStencilInfo.front.compareMask = stateDesc.stencilReadMask;
	m_depthStencilInfo.front.writeMask = stateDesc.stencilWriteMask;
	m_depthStencilInfo.front.reference = 0;
	m_depthStencilInfo.front.compareOp = static_cast<VkCompareOp>(stateDesc.frontFace.stencilFunc);
	m_depthStencilInfo.front.failOp = static_cast<VkStencilOp>(stateDesc.frontFace.stencilFailOp);
	m_depthStencilInfo.front.depthFailOp = static_cast<VkStencilOp>(stateDesc.frontFace.stencilDepthFailOp);
	m_depthStencilInfo.front.passOp = static_cast<VkStencilOp>(stateDesc.frontFace.stencilPassOp);

	m_depthStencilInfo.back.compareMask = stateDesc.stencilReadMask;
	m_depthStencilInfo.back.writeMask = stateDesc.stencilWriteMask;
	m_depthStencilInfo.back.reference = 0;
	m_depthStencilInfo.back.compareOp = static_cast<VkCompareOp>(stateDesc.backFace.stencilFunc);
	m_depthStencilInfo.back.failOp = static_cast<VkStencilOp>(stateDesc.backFace.stencilFailOp);
	m_depthStencilInfo.back.depthFailOp = static_cast<VkStencilOp>(stateDesc.backFace.stencilDepthFailOp);
	m_depthStencilInfo.back.passOp = static_cast<VkStencilOp>(stateDesc.backFace.stencilPassOp);

	// TODO - Add stencil ref to DepthStencilStateDesc
	if (m_depthStencilInfo.stencilTestEnable == VK_TRUE)
	{
		m_dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
	}
}


void GraphicsPSO::SetSampleMask(uint32_t sampleMask)
{
	// TODO hook this up to multisample state
}


void GraphicsPSO::SetPrimitiveTopologyType(PrimitiveTopologyType topologyType)
{
	m_inputAssemblyInfo.topology = static_cast<VkPrimitiveTopology>(topologyType);
}


void GraphicsPSO::SetRenderPass(RenderPass& renderpass)
{ 
	m_renderPass = renderpass.GetRenderPass(); 
}


void GraphicsPSO::SetInputLayout(uint32_t numStreams, const VertexStreamDesc* vertexStreams, uint32_t numElements, const VertexElementDesc* inputElementDescs)
{
	if (numStreams > 0)
	{
		m_vertexInputBindings.resize(numStreams);
		for (uint32_t i = 0; i < numStreams; ++i)
		{
			m_vertexInputBindings[i].binding = vertexStreams[i].inputSlot;
			m_vertexInputBindings[i].inputRate = static_cast<VkVertexInputRate>(vertexStreams[i].inputClassification);
			m_vertexInputBindings[i].stride = vertexStreams[i].stride;
		}
		m_vertexInputInfo.vertexBindingDescriptionCount = numStreams;
		m_vertexInputInfo.pVertexBindingDescriptions = m_vertexInputBindings.data();
	}
	else
	{
		m_vertexInputInfo.vertexBindingDescriptionCount = 0;
		m_vertexInputInfo.pVertexBindingDescriptions = nullptr;
	}

	if (numElements > 0)
	{
		m_vertexAttributes.resize(numElements);
		for (uint32_t i = 0; i < numElements; ++i)
		{
			m_vertexAttributes[i].binding = inputElementDescs[i].inputSlot;
			m_vertexAttributes[i].location = i;
			m_vertexAttributes[i].format = static_cast<VkFormat>(inputElementDescs[i].format);
			m_vertexAttributes[i].offset = inputElementDescs[i].alignedByteOffset;
		}
		m_vertexInputInfo.vertexAttributeDescriptionCount = numElements;
		m_vertexInputInfo.pVertexAttributeDescriptions = m_vertexAttributes.data();
	}
	else
	{
		m_vertexInputInfo.vertexAttributeDescriptionCount = 0;
		m_vertexInputInfo.pVertexAttributeDescriptions = nullptr;
	}
}


void GraphicsPSO::SetPrimitiveRestart(IndexBufferStripCutValue ibProps)
{
	m_inputAssemblyInfo.primitiveRestartEnable = (ibProps == IndexBufferStripCutValue::Disabled) ? VK_FALSE : VK_TRUE;
}


void GraphicsPSO::SetVertexShader(const Shader* vertexShader)
{
	assert(vertexShader != nullptr);
	
	m_vertexShader.binary = vertexShader->GetByteCode();
	m_vertexShader.size = vertexShader->GetByteCodeSize();

	VkShaderModuleCreateInfo moduleCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = m_vertexShader.size;
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_vertexShader.binary);

	VkShaderModule module{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateShaderModule(GetDevice(), &moduleCreateInfo, nullptr, &module));

	VkPipelineShaderStageCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
	createInfo.pName = "main";
	createInfo.module = module;
	createInfo.pSpecializationInfo = nullptr;

	m_shaderStages.push_back(createInfo);
}


void GraphicsPSO::SetPixelShader(const Shader* pixelShader)
{
	m_pixelShader.binary = pixelShader->GetByteCode();
	m_pixelShader.size = pixelShader->GetByteCodeSize();

	VkShaderModuleCreateInfo moduleCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = m_pixelShader.size;
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_pixelShader.binary);

	VkShaderModule module{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateShaderModule(GetDevice(), &moduleCreateInfo, nullptr, &module));

	VkPipelineShaderStageCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	createInfo.pName = "main";
	createInfo.module = module;
	createInfo.pSpecializationInfo = nullptr;

	m_shaderStages.push_back(createInfo);
}


void GraphicsPSO::SetHullShader(const Shader* hullShader)
{
	m_hullShader.binary = hullShader->GetByteCode();
	m_hullShader.size = hullShader->GetByteCodeSize();

	VkShaderModuleCreateInfo moduleCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = m_hullShader.size;
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_hullShader.binary);

	VkShaderModule module{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateShaderModule(GetDevice(), &moduleCreateInfo, nullptr, &module));

	VkPipelineShaderStageCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
	createInfo.pName = "main";
	createInfo.module = module;
	createInfo.pSpecializationInfo = nullptr;

	m_shaderStages.push_back(createInfo);
}


void GraphicsPSO::SetDomainShader(const Shader* domainShader)
{
	m_domainShader.binary = domainShader->GetByteCode();
	m_domainShader.size = domainShader->GetByteCodeSize();

	VkShaderModuleCreateInfo moduleCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = m_domainShader.size;
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_domainShader.binary);

	VkShaderModule module{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateShaderModule(GetDevice(), &moduleCreateInfo, nullptr, &module));

	VkPipelineShaderStageCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
	createInfo.pName = "main";
	createInfo.module = module;
	createInfo.pSpecializationInfo = nullptr;

	m_shaderStages.push_back(createInfo);
}


void GraphicsPSO::SetGeometryShader(const Shader* geometryShader)
{
	m_geometryShader.binary = geometryShader->GetByteCode();
	m_geometryShader.size = geometryShader->GetByteCodeSize();

	VkShaderModuleCreateInfo moduleCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = m_geometryShader.size;
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_geometryShader.binary);

	VkShaderModule module{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateShaderModule(GetDevice(), &moduleCreateInfo, nullptr, &module));

	VkPipelineShaderStageCreateInfo createInfo{ VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
	createInfo.pName = "main";
	createInfo.module = module;
	createInfo.pSpecializationInfo = nullptr;

	m_shaderStages.push_back(createInfo);
}


void GraphicsPSO::Finalize()
{
	VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stageCount = static_cast<uint32_t>(m_shaderStages.size());
	createInfo.pStages = m_shaderStages.empty() ? nullptr : m_shaderStages.data();
	createInfo.pVertexInputState = &m_vertexInputInfo;
	createInfo.pInputAssemblyState = &m_inputAssemblyInfo;
	createInfo.pTessellationState = nullptr;
	createInfo.pViewportState = &m_viewportStateInfo;
	createInfo.pRasterizationState = &m_rasterizationInfo;
	createInfo.pMultisampleState = &m_multisampleInfo;
	createInfo.pDepthStencilState = &m_depthStencilInfo;
	createInfo.pColorBlendState = &m_blendStateInfo;
	// TODO
#if 0
	createInfo.layout = m_rootSignature->GetLayout();
#else
	createInfo.layout = m_layout;
#endif
	createInfo.renderPass = m_renderPass;
	createInfo.subpass = 0;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = 0;

	VkPipelineDynamicStateCreateInfo dynamicStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateInfo.pNext = nullptr;
	dynamicStateInfo.flags = 0;
	dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(m_dynamicStates.size());
	dynamicStateInfo.pDynamicStates = m_dynamicStates.empty() ? nullptr : m_dynamicStates.data();

	createInfo.pDynamicState = &dynamicStateInfo;

	{
		lock_guard<mutex> CS(s_pipelineMutex);

		if (s_pipelineCache == VK_NULL_HANDLE)
		{
			VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
			pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
			ThrowIfFailed(vkCreatePipelineCache(GetDevice(), &pipelineCacheCreateInfo, nullptr, &s_pipelineCache));
		}

		ThrowIfFailed(vkCreateGraphicsPipelines(GetDevice(), s_pipelineCache, 1, &createInfo, nullptr, &m_pipeline));

		s_graphicsPipelineCache.insert(m_pipeline);
	}

	// Clean up shader modules
	for (auto& shaderStage : m_shaderStages)
	{
		vkDestroyShaderModule(GetDevice(), shaderStage.module, nullptr);
	}
	m_shaderStages.clear();
}


ComputePSO::ComputePSO()
{
}


void ComputePSO::Finalize()
{
}