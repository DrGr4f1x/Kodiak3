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

#include "Hash.h"

#include "GraphicsDeviceVk.h"


using namespace Kodiak;
using namespace std;


namespace
{
map<size_t, VkPipelineLayout> s_pipelineLayoutHashMap;
}


RootParameter::~RootParameter()
{
	auto device = GetDevice();

	vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
	m_descriptorSetLayout = VK_NULL_HANDLE;
}


void RootSignature::DestroyAll()
{
	auto device = GetDevice();

	for (auto& layoutPair : s_pipelineLayoutHashMap)
	{
		vkDestroyPipelineLayout(device, layoutPair.second, nullptr);
	}
	s_pipelineLayoutHashMap.clear();
}


void RootSignature::Destroy()
{
	m_paramArray.reset(nullptr);
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

		auto device = GetDevice();

		// Gather the descriptor layouts and push constants
		for (uint32_t i = 0; i < m_numParameters; ++i)
		{
			auto& parameter = m_paramArray[i];

			VkDescriptorSetLayoutCreateInfo createInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
			createInfo.pNext = nullptr;
			createInfo.flags = 0;
			createInfo.bindingCount = static_cast<uint32_t>(parameter.m_bindings.size());
			createInfo.pBindings = parameter.m_bindings.empty() ? nullptr : parameter.m_bindings.data();

			ThrowIfFailed(vkCreateDescriptorSetLayout(device, &createInfo, nullptr, &parameter.m_descriptorSetLayout));

			descriptorSetLayouts.push_back(parameter.m_descriptorSetLayout);
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