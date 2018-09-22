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

#include "RootSignatureVk.h"

#include "DescriptorHeap.h"
#include "Hash.h"
#include "SamplerState.h"

#include "GraphicsDevice.h"


using namespace Kodiak;
using namespace std;


namespace
{

map<size_t, VkPipelineLayout> s_pipelineLayoutHashMap;


VkFilter GetMinFilter(TextureFilter filter)
{
	switch (filter)
	{
	case TextureFilter::MinMagMipPoint:
	case TextureFilter::MinMagPointMipLinear:
	case TextureFilter::MinPointMagLinearMipPoint:
	case TextureFilter::MinPointMagMipLinear:
	case TextureFilter::ComparisonMinMagMipPoint:
	case TextureFilter::ComparisonMinMagPointMipLinear:
	case TextureFilter::ComparisonMinPointMagLinearMipPoint:
	case TextureFilter::ComparisonMinPointMagMipLinear:
	case TextureFilter::MinimumMinMagMipPoint:
	case TextureFilter::MinimumMinMagPointMipLinear:
	case TextureFilter::MinimumMinPointMagLinearMipPoint:
	case TextureFilter::MinimumMinPointMagMipLinear:
	case TextureFilter::MaximumMinMagMipPoint:
	case TextureFilter::MaximumMinMagPointMipLinear:
	case TextureFilter::MaximumMinPointMagLinearMipPoint:
	case TextureFilter::MaximumMinPointMagMipLinear:
		return VK_FILTER_NEAREST;

	default:
		return VK_FILTER_LINEAR;
	}
}


VkFilter GetMagFilter(TextureFilter filter)
{
	switch (filter)
	{
	case TextureFilter::MinMagMipPoint:
	case TextureFilter::MinMagPointMipLinear:
	case TextureFilter::MinLinearMagMipPoint:
	case TextureFilter::MinLinearMagPointMipLinear:
	case TextureFilter::ComparisonMinMagMipPoint:
	case TextureFilter::ComparisonMinMagPointMipLinear:
	case TextureFilter::ComparisonMinLinearMagMipPoint:
	case TextureFilter::ComparisonMinLinearMagPointMipLinear:
	case TextureFilter::MinimumMinMagMipPoint:
	case TextureFilter::MinimumMinMagPointMipLinear:
	case TextureFilter::MinimumMinLinearMagMipPoint:
	case TextureFilter::MinimumMinLinearMagPointMipLinear:
	case TextureFilter::MaximumMinMagMipPoint:
	case TextureFilter::MaximumMinMagPointMipLinear:
	case TextureFilter::MaximumMinLinearMagMipPoint:
	case TextureFilter::MaximumMinLinearMagPointMipLinear:
		return VK_FILTER_NEAREST;

	default:
		return VK_FILTER_LINEAR;
	}
}


VkSamplerMipmapMode GetMipFilter(TextureFilter filter)
{
	switch (filter)
	{
	case TextureFilter::MinMagMipPoint:
	case TextureFilter::MinPointMagLinearMipPoint:
	case TextureFilter::MinLinearMagMipPoint:
	case TextureFilter::MinMagLinearMipPoint:
	case TextureFilter::ComparisonMinMagMipPoint:
	case TextureFilter::ComparisonMinPointMagLinearMipPoint:
	case TextureFilter::ComparisonMinLinearMagMipPoint:
	case TextureFilter::ComparisonMinMagLinearMipPoint:
	case TextureFilter::MinimumMinMagMipPoint:
	case TextureFilter::MinimumMinPointMagLinearMipPoint:
	case TextureFilter::MinimumMinLinearMagMipPoint:
	case TextureFilter::MinimumMinMagLinearMipPoint:
	case TextureFilter::MaximumMinMagMipPoint:
	case TextureFilter::MaximumMinPointMagLinearMipPoint:
	case TextureFilter::MaximumMinLinearMagMipPoint:
	case TextureFilter::MaximumMinMagLinearMipPoint:
		return VK_SAMPLER_MIPMAP_MODE_NEAREST;

	default:
		return VK_SAMPLER_MIPMAP_MODE_LINEAR;
	}
}


VkBool32 IsAnisotropic(TextureFilter filter)
{
	switch (filter)
	{
	case TextureFilter::Anisotropic:
	case TextureFilter::ComparisonAnisotropic:
	case TextureFilter::MinimumAnisotropic:
	case TextureFilter::MaximumAnisotropic:
		return VK_TRUE;

	default:
		return VK_FALSE;
	}
}


VkBool32 IsComparisonEnabled(TextureFilter filter)
{
	switch (filter)
	{
	case TextureFilter::ComparisonMinMagMipPoint:
	case TextureFilter::ComparisonMinMagPointMipLinear:
	case TextureFilter::ComparisonMinPointMagLinearMipPoint:
	case TextureFilter::ComparisonMinPointMagMipLinear:
	case TextureFilter::ComparisonMinLinearMagMipPoint:
	case TextureFilter::ComparisonMinLinearMagPointMipLinear:
	case TextureFilter::ComparisonMinMagLinearMipPoint:
	case TextureFilter::ComparisonMinMagMipLinear:
	case TextureFilter::ComparisonAnisotropic:
		return VK_TRUE;

	default:
		return VK_FALSE;
	}
}

} // anonymous namespace


RootParameter::~RootParameter()
{
	VkDevice device = GetDevice();

	vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
	m_descriptorSetLayout = VK_NULL_HANDLE;
}

void RootSignature::DestroyAll()
{
	VkDevice device = GetDevice();

	for (auto& layoutPair : s_pipelineLayoutHashMap)
	{
		vkDestroyPipelineLayout(device, layoutPair.second, nullptr);
	}
	s_pipelineLayoutHashMap.clear();
}


void RootSignature::Destroy()
{
	m_paramArray.reset(nullptr);

	VkDevice device = GetDevice();

	vkDestroyDescriptorSetLayout(device, m_samplerLayout, nullptr);

	// Destroy samplers
	for (auto& sampler : m_samplers)
	{
		vkDestroySampler(device, sampler, nullptr);
	}
}


void RootSignature::InitStaticSampler(uint32_t _register, const SamplerStateDesc& nonStaticSamplerDesc,
	ShaderVisibility visibility)
{
	assert(m_numInitializedStaticSamplers < m_numSamplers);
	SamplerInfo& samplerInfo = m_samplerArray[m_numInitializedStaticSamplers++];

	samplerInfo._register = _register;
	samplerInfo.visibility = visibility;

	samplerInfo.createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.createInfo.pNext = nullptr;
	samplerInfo.createInfo.flags = 0;

	samplerInfo.createInfo.maxAnisotropy = 1.0f;
	samplerInfo.createInfo.minFilter = GetMinFilter(nonStaticSamplerDesc.filter);
	samplerInfo.createInfo.magFilter = GetMagFilter(nonStaticSamplerDesc.filter);
	samplerInfo.createInfo.mipmapMode = GetMipFilter(nonStaticSamplerDesc.filter);
	samplerInfo.createInfo.addressModeU = static_cast<VkSamplerAddressMode>(nonStaticSamplerDesc.addressU);
	samplerInfo.createInfo.addressModeV = static_cast<VkSamplerAddressMode>(nonStaticSamplerDesc.addressV);
	samplerInfo.createInfo.addressModeW = static_cast<VkSamplerAddressMode>(nonStaticSamplerDesc.addressW);
	samplerInfo.createInfo.mipLodBias = nonStaticSamplerDesc.mipLODBias;
	samplerInfo.createInfo.minLod = nonStaticSamplerDesc.minLOD;
	samplerInfo.createInfo.maxLod = nonStaticSamplerDesc.maxLOD;
	samplerInfo.createInfo.anisotropyEnable = IsAnisotropic(nonStaticSamplerDesc.filter);
	samplerInfo.createInfo.compareEnable = IsComparisonEnabled(nonStaticSamplerDesc.filter);
	samplerInfo.createInfo.compareOp = static_cast<VkCompareOp>(nonStaticSamplerDesc.comparisonFunc);
	samplerInfo.createInfo.unnormalizedCoordinates = VK_FALSE;
	samplerInfo.createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;

	if (samplerInfo.createInfo.addressModeU == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
		samplerInfo.createInfo.addressModeV == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER ||
		samplerInfo.createInfo.addressModeW == VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER)
	{
		warn_once_if_not(
			// Transparent Black
			nonStaticSamplerDesc.borderColor[0] == 0.0f &&
			nonStaticSamplerDesc.borderColor[1] == 0.0f &&
			nonStaticSamplerDesc.borderColor[2] == 0.0f &&
			nonStaticSamplerDesc.borderColor[3] == 0.0f ||
			// Opaque Black
			nonStaticSamplerDesc.borderColor[0] == 0.0f &&
			nonStaticSamplerDesc.borderColor[1] == 0.0f &&
			nonStaticSamplerDesc.borderColor[2] == 0.0f &&
			nonStaticSamplerDesc.borderColor[3] == 1.0f ||
			// Opaque White
			nonStaticSamplerDesc.borderColor[0] == 1.0f &&
			nonStaticSamplerDesc.borderColor[1] == 1.0f &&
			nonStaticSamplerDesc.borderColor[2] == 1.0f &&
			nonStaticSamplerDesc.borderColor[3] == 1.0f,
			"Sampler border color does not match static sampler limitations");

		if (nonStaticSamplerDesc.borderColor[3] == 1.0f)
		{
			if (nonStaticSamplerDesc.borderColor[0] == 1.0f)
			{
				samplerInfo.createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			}
			else
			{
				samplerInfo.createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			}
		}
		else
		{
			samplerInfo.createInfo.borderColor = VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		}
	}
}


void RootSignature::Finalize(const string& name, RootSignatureFlags flags)
{
	if (m_finalized)
	{
		return;
	}

	size_t hashCode = Utility::g_hashStart;

	// Hash the parameters
	for (uint32_t i = 0; i < m_numParameters; ++i)
	{
		auto& parameter = m_paramArray[i];
		for (const auto& binding : parameter.m_bindings)
		{
			hashCode = Utility::HashState(&binding, 1, hashCode);
		}
	}

	VkPipelineLayout* layoutRef = nullptr;
	bool firstCompile = false;
	{
		static mutex s_hashMapMutex;
		lock_guard<mutex> CS(s_hashMapMutex);

		auto iter = s_pipelineLayoutHashMap.find(hashCode);

		// Reserve space so the next inquiry will find that someone got here first.
		if (iter == s_pipelineLayoutHashMap.end())
		{
			s_pipelineLayoutHashMap[hashCode] = VK_NULL_HANDLE;
			layoutRef = &s_pipelineLayoutHashMap[hashCode];
			firstCompile = true;
		}
		else
		{
			layoutRef = &iter->second;
		}
	}

	if (firstCompile)
	{
		vector<VkDescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.reserve(m_numParameters);

		VkDevice device = GetDevice();

		// Gather the descriptor layouts and push constants
		for (uint32_t i = 0; i < m_numParameters; ++i)
		{
			auto& parameter = m_paramArray[i];

			const bool usePushDescriptor = parameter.m_type == RootParameterType::RootCBV || parameter.m_type == RootParameterType::DynamicRootCBV;

			VkDescriptorSetLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			createInfo.pNext = nullptr;
			createInfo.flags = usePushDescriptor ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR : 0;
			createInfo.bindingCount = static_cast<uint32_t>(parameter.m_bindings.size());
			createInfo.pBindings = parameter.m_bindings.empty() ? nullptr : parameter.m_bindings.data();

			ThrowIfFailed(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &parameter.m_descriptorSetLayout));

			descriptorSetLayouts.push_back(parameter.m_descriptorSetLayout);
		}

		// Create static samplers
		m_samplers.resize(m_numSamplers);

		if (m_numSamplers > 0)
		{
			VkDescriptorSetLayoutBinding samplerBinding;
			samplerBinding.binding = 0;
			samplerBinding.descriptorCount = m_numSamplers;
			samplerBinding.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			samplerBinding.stageFlags = VK_SHADER_STAGE_ALL;

			// TODO
			// This code assumes samplers are contiguous from 0 to m_numSamplers-1 and all have the same
			// shader visibility.  A better system would create ranges of contiguous sampler bindings
			// per shader visibility type.

			for (uint32_t i = 0; i < m_numSamplers; ++i)
			{
				ThrowIfFailed(vkCreateSampler(device, &m_samplerArray[i].createInfo, nullptr, &m_samplers[i]));
			}

			samplerBinding.pImmutableSamplers = m_samplers.data();

			VkDescriptorSetLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.bindingCount = 1;
			createInfo.pBindings = &samplerBinding;

			ThrowIfFailed(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &m_samplerLayout));

			m_staticSamplerSetIndex = static_cast<uint32_t>(descriptorSetLayouts.size());

			descriptorSetLayouts.push_back(m_samplerLayout);

			// Finally, create a dummy descriptor set for the static samplers
			m_staticSamplerSet = AllocateDescriptorSet(m_samplerLayout);
		}

		// Create the pipeline layout
		VkPipelineLayoutCreateInfo pipelineInfo{ VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO };
		pipelineInfo.pNext = nullptr;
		pipelineInfo.flags = 0;
		pipelineInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineInfo.pSetLayouts = descriptorSetLayouts.empty() ? nullptr : descriptorSetLayouts.data();
		pipelineInfo.pushConstantRangeCount = 0;
		pipelineInfo.pPushConstantRanges = nullptr;

		ThrowIfFailed(vkCreatePipelineLayout(device, &pipelineInfo, nullptr, &m_layout));

		s_pipelineLayoutHashMap[hashCode] = m_layout;

		assert(*layoutRef == m_layout);
	}
	else
	{
		while (*layoutRef == VK_NULL_HANDLE)
		{
			this_thread::yield();
		}
		m_layout = *layoutRef;
	}

	m_finalized = true;
}