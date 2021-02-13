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

#include "Graphics\PipelineState.h"

#include "Hash.h"
#include "Graphics\GraphicsDevice.h"
#include "Graphics\PixelBuffer.h"
#include "Graphics\Shader.h"

#include "RootSignatureVk.h"
#include "UtilVk.h"


using namespace Kodiak;
using namespace std;


void PSO::DestroyAll()
{
	// TODO Remove this function
}


namespace
{

void CreateShaderStageInfo(VkPipelineShaderStageCreateInfo& createInfo, const Shader* shader, VkShaderStageFlagBits shaderStage)
{
	VkDevice device = GetDevice();

	VkShaderModuleCreateInfo moduleCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = shader->GetByteCodeSize();
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(shader->GetByteCode());

	VkShaderModule shaderModule{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &shaderModule));

	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stage = shaderStage;
	createInfo.pName = "main";
	createInfo.module = shaderModule;
	createInfo.pSpecializationInfo = nullptr;
}

} // anonymous namespace


void GraphicsPSO::SetParent(GraphicsPSO* pso)
{
	m_parentPSO = pso;
	m_parentPSO->m_isParent = true;
}


void GraphicsPSO::Finalize()
{
	vector<VkPipelineShaderStageCreateInfo> shaderStages;

	VkDevice device = GetDevice();

	vector<VkDynamicState> dynamicStates;
	dynamicStates.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamicStates.push_back(VK_DYNAMIC_STATE_SCISSOR);

	// Blend state
	VkPipelineColorBlendStateCreateInfo blendStateInfo = { VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO };
	blendStateInfo.pNext = nullptr;
	blendStateInfo.flags = 0;

	array<VkPipelineColorBlendAttachmentState, 8> blendAttachments;

	VkPipelineMultisampleStateCreateInfo multisampleInfo = { VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO };
	multisampleInfo.pNext = nullptr;
	multisampleInfo.flags = 0;
	multisampleInfo.sampleShadingEnable = VK_FALSE;
	multisampleInfo.alphaToOneEnable = VK_FALSE;
	multisampleInfo.alphaToCoverageEnable = VK_FALSE;
	uint32_t smplMask = 0x1;
	multisampleInfo.pSampleMask = nullptr;
	multisampleInfo.minSampleShading = 0.0f;
	multisampleInfo.rasterizationSamples = SamplesToFlags(m_msaaCount);

	for (uint32_t i = 0; i < 8; ++i)
	{
		const auto& rt = m_blendState.renderTargetBlend[i];
		blendAttachments[i].blendEnable = rt.blendEnable ? VK_TRUE : VK_FALSE;
		blendAttachments[i].srcColorBlendFactor = static_cast<VkBlendFactor>(rt.srcBlend);
		blendAttachments[i].dstColorBlendFactor = static_cast<VkBlendFactor>(rt.dstBlend);
		blendAttachments[i].colorBlendOp = static_cast<VkBlendOp>(rt.blendOp);
		blendAttachments[i].srcAlphaBlendFactor = static_cast<VkBlendFactor>(rt.srcBlendAlpha);
		blendAttachments[i].dstAlphaBlendFactor = static_cast<VkBlendFactor>(rt.dstBlendAlpha);
		blendAttachments[i].alphaBlendOp = static_cast<VkBlendOp>(rt.blendOpAlpha);
		blendAttachments[i].colorWriteMask = static_cast<VkColorComponentFlags>(rt.writeMask);

		// First render target with logic op enabled gets to set the state
		if (rt.logicOpEnable && (VK_FALSE == blendStateInfo.logicOpEnable))
		{
			blendStateInfo.logicOpEnable = VK_TRUE;
			blendStateInfo.logicOp = static_cast<VkLogicOp>(rt.logicOp);
		}
	}
	blendStateInfo.attachmentCount = m_numRtvs;
	blendStateInfo.pAttachments = blendAttachments.data();

	multisampleInfo.alphaToCoverageEnable = m_blendState.alphaToCoverageEnable ? VK_TRUE : VK_FALSE;

	// Rasterizer state
	VkPipelineRasterizationStateCreateInfo rasterizerInfo = { VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO };
	rasterizerInfo.flags = 0;
	rasterizerInfo.lineWidth = 1.0f;
	rasterizerInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizerInfo.depthClampEnable = VK_FALSE;

	rasterizerInfo.polygonMode = static_cast<VkPolygonMode>(m_rasterizerState.fillMode);
	rasterizerInfo.cullMode = static_cast<VkCullModeFlags>(m_rasterizerState.cullMode);
	rasterizerInfo.frontFace = m_rasterizerState.frontCounterClockwise ? VK_FRONT_FACE_COUNTER_CLOCKWISE : VK_FRONT_FACE_CLOCKWISE;
	rasterizerInfo.depthBiasEnable = (m_rasterizerState.depthBias != 0 || m_rasterizerState.slopeScaledDepthBias != 0.0f) ? VK_TRUE : VK_FALSE;
	rasterizerInfo.depthBiasClamp = m_rasterizerState.depthBiasClamp;
	rasterizerInfo.depthBiasConstantFactor = *reinterpret_cast<const float*>(&m_rasterizerState.depthBias);
	rasterizerInfo.depthBiasSlopeFactor = m_rasterizerState.slopeScaledDepthBias;

	// Depth-stencil state
	VkPipelineDepthStencilStateCreateInfo depthStencilInfo = { VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO };
	depthStencilInfo.pNext = nullptr;
	depthStencilInfo.flags = 0;
	depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilInfo.minDepthBounds = 0.0f;
	depthStencilInfo.maxDepthBounds = 1.0f;

	depthStencilInfo.depthTestEnable = m_depthStencilState.depthEnable ? VK_TRUE : VK_FALSE;
	depthStencilInfo.depthWriteEnable = (m_depthStencilState.depthWriteMask == DepthWrite::All) ? VK_TRUE : VK_FALSE;
	depthStencilInfo.depthCompareOp = static_cast<VkCompareOp>(m_depthStencilState.depthFunc);
	depthStencilInfo.stencilTestEnable = m_depthStencilState.stencilEnable ? VK_TRUE : VK_FALSE;

	depthStencilInfo.front.compareMask = m_depthStencilState.stencilReadMask;
	depthStencilInfo.front.writeMask = m_depthStencilState.stencilWriteMask;
	depthStencilInfo.front.reference = 0;
	depthStencilInfo.front.compareOp = static_cast<VkCompareOp>(m_depthStencilState.frontFace.stencilFunc);
	depthStencilInfo.front.failOp = static_cast<VkStencilOp>(m_depthStencilState.frontFace.stencilFailOp);
	depthStencilInfo.front.depthFailOp = static_cast<VkStencilOp>(m_depthStencilState.frontFace.stencilDepthFailOp);
	depthStencilInfo.front.passOp = static_cast<VkStencilOp>(m_depthStencilState.frontFace.stencilPassOp);

	depthStencilInfo.back.compareMask = m_depthStencilState.stencilReadMask;
	depthStencilInfo.back.writeMask = m_depthStencilState.stencilWriteMask;
	depthStencilInfo.back.reference = 0;
	depthStencilInfo.back.compareOp = static_cast<VkCompareOp>(m_depthStencilState.backFace.stencilFunc);
	depthStencilInfo.back.failOp = static_cast<VkStencilOp>(m_depthStencilState.backFace.stencilFailOp);
	depthStencilInfo.back.depthFailOp = static_cast<VkStencilOp>(m_depthStencilState.backFace.stencilDepthFailOp);
	depthStencilInfo.back.passOp = static_cast<VkStencilOp>(m_depthStencilState.backFace.stencilPassOp);

	// TODO - Add stencil ref to DepthStencilStateDesc
	if (depthStencilInfo.stencilTestEnable == VK_TRUE)
	{
		dynamicStates.push_back(VK_DYNAMIC_STATE_STENCIL_REFERENCE);
	}

	// Primitive topology & primitive restart
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo = { VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO };
	inputAssemblyInfo.pNext = nullptr;
	inputAssemblyInfo.flags = 0;

	VkPipelineTessellationStateCreateInfo tessellationInfo = { VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO };
	tessellationInfo.pNext = nullptr;
	tessellationInfo.flags = 0;
	tessellationInfo.patchControlPoints = 0;

	inputAssemblyInfo.topology = MapEnginePrimitiveTopologyToVulkan(m_topology);
	inputAssemblyInfo.primitiveRestartEnable = (m_ibStripCut == IndexBufferStripCutValue::Disabled) ? VK_FALSE : VK_TRUE;
	tessellationInfo.patchControlPoints = GetControlPointCount(m_topology);

	// Input layout
	VkPipelineVertexInputStateCreateInfo vertexInputInfo{ VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO };
	vertexInputInfo.pNext = nullptr;
	vertexInputInfo.flags = 0;

	vector<VkVertexInputBindingDescription> vertexInputBindings;
	vector<VkVertexInputAttributeDescription> vertexAttributes;

	const uint32_t numStreams = (uint32_t)m_vertexStreams.size();
	if (numStreams > 0)
	{
		vertexInputBindings.resize(numStreams);
		for (uint32_t i = 0; i < numStreams; ++i)
		{
			vertexInputBindings[i].binding = m_vertexStreams[i].inputSlot;
			vertexInputBindings[i].inputRate = static_cast<VkVertexInputRate>(m_vertexStreams[i].inputClassification);
			vertexInputBindings[i].stride = m_vertexStreams[i].stride;
		}
		vertexInputInfo.vertexBindingDescriptionCount = numStreams;
		vertexInputInfo.pVertexBindingDescriptions = vertexInputBindings.data();
	}
	else
	{
		vertexInputInfo.vertexBindingDescriptionCount = 0;
		vertexInputInfo.pVertexBindingDescriptions = nullptr;
	}

	const uint32_t numElements = (uint32_t)m_vertexElements.size();
	if (numElements > 0)
	{
		vertexAttributes.resize(numElements);
		for (uint32_t i = 0; i < numElements; ++i)
		{
			vertexAttributes[i].binding = m_vertexElements[i].inputSlot;
			vertexAttributes[i].location = i;
			vertexAttributes[i].format = static_cast<VkFormat>(m_vertexElements[i].format);
			vertexAttributes[i].offset = m_vertexElements[i].alignedByteOffset;
		}
		vertexInputInfo.vertexAttributeDescriptionCount = numElements;
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributes.data();
	}
	else
	{
		vertexInputInfo.vertexAttributeDescriptionCount = 0;
		vertexInputInfo.pVertexAttributeDescriptions = nullptr;
	}

	// Vertex shader
	if (m_vertexShader)
	{
		VkPipelineShaderStageCreateInfo createInfo;
		CreateShaderStageInfo(createInfo, m_vertexShader, VK_SHADER_STAGE_VERTEX_BIT);

		shaderStages.push_back(createInfo);
	}

	// Pixel shader
	if (m_pixelShader)
	{
		VkPipelineShaderStageCreateInfo createInfo;
		CreateShaderStageInfo(createInfo, m_pixelShader, VK_SHADER_STAGE_FRAGMENT_BIT);

		shaderStages.push_back(createInfo);
	}

	// Hull shader
	if (m_hullShader)
	{
		VkPipelineShaderStageCreateInfo createInfo;
		CreateShaderStageInfo(createInfo, m_hullShader, VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
		
		shaderStages.push_back(createInfo);
	}

	// Domain shader
	if(m_domainShader)
	{
		VkPipelineShaderStageCreateInfo createInfo;
		CreateShaderStageInfo(createInfo, m_domainShader, VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
		
		shaderStages.push_back(createInfo);
	}

	// Geometry shader
	if (m_geometryShader)
	{
		VkPipelineShaderStageCreateInfo createInfo;
		CreateShaderStageInfo(createInfo, m_geometryShader, VK_SHADER_STAGE_GEOMETRY_BIT);
		
		shaderStages.push_back(createInfo);
	}

	// Viewport state
	VkPipelineViewportStateCreateInfo viewportStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO };
	viewportStateInfo.pNext = nullptr;
	viewportStateInfo.flags = 0;
	viewportStateInfo.viewportCount = 1;
	viewportStateInfo.scissorCount = 1;

	// Temp render pass
	VkRenderPass renderpass{ VK_NULL_HANDLE };
	{
		VkRenderPassCreateInfo rpInfo = {};
		rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		rpInfo.pNext = 0;
		rpInfo.flags = 0;

		vector<VkAttachmentDescription> attachments(m_numRtvs + 1);
		vector<uint32_t> regToAttachmentIndex(attachments.size(), VK_ATTACHMENT_UNUSED);

		uint32_t rtCount = 0;

		// Process color targets
		for (uint32_t i = 0; i < m_numRtvs; ++i)
		{
			auto format = m_rtvFormats[i];
			if (format != Format::Unknown)
			{
				auto& desc = attachments[rtCount];
				regToAttachmentIndex[i] = rtCount;
				++rtCount;

				desc.flags = 0;
				desc.format = static_cast<VkFormat>(format);
				desc.samples = SamplesToFlags(m_msaaCount);
				desc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
				desc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
				desc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE; // This is a color attachment
				desc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // This is a color attachment
				desc.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
				desc.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}
		}

		bool hasColor = rtCount > 0;
		bool hasDepth = false;

		// Process depth target
		if (m_dsvFormat != Format::Unknown)
		{
			auto& depthDesc = attachments[rtCount];
			regToAttachmentIndex.back() = rtCount;
			rtCount++;

			depthDesc.flags = 0;
			depthDesc.format = static_cast<VkFormat>(m_dsvFormat);
			depthDesc.samples = SamplesToFlags(m_msaaCount);
			depthDesc.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthDesc.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthDesc.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			depthDesc.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			depthDesc.initialLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			depthDesc.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			hasDepth = true;
		}

		// Init subpass
		VkSubpassDescription subpassDesc = {};
		subpassDesc.flags = 0;

		vector<VkAttachmentReference> attachmentRefs(attachments.size());

		if (hasDepth)
		{
			auto& depthRef = attachmentRefs.back();
			depthRef.attachment = regToAttachmentIndex.back();
			depthRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
			subpassDesc.pDepthStencilAttachment = &attachmentRefs.back();
		}

		if (hasColor)
		{
			for (uint32_t i = 0; i < m_numRtvs; ++i)
			{
				auto& ref = attachmentRefs[i];
				ref.attachment = regToAttachmentIndex[i];
				ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			}

			subpassDesc.colorAttachmentCount = m_numRtvs;
			subpassDesc.pColorAttachments = attachmentRefs.data();
		}

		// Build renderpass
		VkRenderPassCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.attachmentCount = rtCount;
		createInfo.pAttachments = attachments.data();
		createInfo.subpassCount = 1;
		createInfo.pSubpasses = &subpassDesc;
		createInfo.dependencyCount = 0;
		createInfo.pDependencies = nullptr;

		ThrowIfFailed(vkCreateRenderPass(GetDevice(), &createInfo, nullptr, &renderpass));
	}

	VkGraphicsPipelineCreateInfo createInfo{ VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO };
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.stageCount = static_cast<uint32_t>(shaderStages.size());
	createInfo.pStages = shaderStages.empty() ? nullptr : shaderStages.data();
	createInfo.pVertexInputState = &vertexInputInfo;
	createInfo.pInputAssemblyState = &inputAssemblyInfo;
	createInfo.pTessellationState = &tessellationInfo;
	createInfo.pViewportState = &viewportStateInfo;
	createInfo.pRasterizationState = &rasterizerInfo;
	createInfo.pMultisampleState = &multisampleInfo;
	createInfo.pDepthStencilState = &depthStencilInfo;
	createInfo.pColorBlendState = &blendStateInfo;
	createInfo.layout = m_rootSignature->GetLayout();
	createInfo.renderPass = renderpass;
	createInfo.subpass = 0;
	createInfo.basePipelineHandle = VK_NULL_HANDLE;
	createInfo.basePipelineIndex = 0;

	if (m_isParent)
	{
		createInfo.flags = VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT;
	}

	if (m_parentPSO != nullptr)
	{
		assert_msg(
			m_parentPSO->GetPipeline() != VK_NULL_HANDLE,
			"Parent PSO has NULL handle.  Make sure to call Finalize() on the parent PSO before calling Finalize() on child PSOs!");

		createInfo.basePipelineHandle = m_parentPSO->GetPipeline();
		createInfo.basePipelineIndex = -1;
	}

	VkPipelineDynamicStateCreateInfo dynamicStateInfo{ VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO };
	dynamicStateInfo.pNext = nullptr;
	dynamicStateInfo.flags = 0;
	dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
	dynamicStateInfo.pDynamicStates = dynamicStates.empty() ? nullptr : dynamicStates.data();

	createInfo.pDynamicState = &dynamicStateInfo;

	ThrowIfFailed(g_graphicsDevice->CreateGraphicsPipeline(createInfo, &m_pipeline));

	vkDestroyRenderPass(device, renderpass, nullptr);

	// Clean up shader modules
	for (auto& shaderStage : shaderStages)
	{
		vkDestroyShaderModule(device, shaderStage.module, nullptr);
	}
	shaderStages.clear();
}


void ComputePSO::Finalize()
{
	VkDevice device = GetDevice();

	VkComputePipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	createInfo.pNext = nullptr;
	createInfo.flags = 0;
	createInfo.layout = m_rootSignature->GetLayout();

	VkShaderModuleCreateInfo moduleCreateInfo{ VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO };
	moduleCreateInfo.pNext = nullptr;
	moduleCreateInfo.flags = 0;
	moduleCreateInfo.codeSize = m_computeShader->GetByteCodeSize();
	moduleCreateInfo.pCode = reinterpret_cast<const uint32_t*>(m_computeShader->GetByteCode());

	VkShaderModule module{ VK_NULL_HANDLE };
	ThrowIfFailed(vkCreateShaderModule(device, &moduleCreateInfo, nullptr, &module));

	createInfo.stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	createInfo.stage.pNext = nullptr;
	createInfo.stage.flags = 0;
	createInfo.stage.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	createInfo.stage.pName = "main";
	createInfo.stage.module = module;
	createInfo.stage.pSpecializationInfo = nullptr;

	ThrowIfFailed(g_graphicsDevice->CreateComputePipeline(createInfo, &m_pipeline));

	vkDestroyShaderModule(device, module, nullptr);
}