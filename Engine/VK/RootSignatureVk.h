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

namespace Kodiak
{

// Forward declarations
struct SamplerStateDesc;


struct DescriptorRange
{
	DescriptorType type;
	uint32_t numDescriptors{ 0 };
};


class RootParameter
{
	friend class RootSignature;

public:
	RootParameter() = default;

	~RootParameter();

	void InitAsConstantBuffer(uint32_t _register, ShaderVisibility visibility = ShaderVisibility::All)
	{
		assert(m_type == RootParameterType::Invalid);
		assert(m_bindings.empty());

		m_type = RootParameterType::CBV;
		m_visibility = visibility;

		VkDescriptorSetLayoutBinding binding;
		binding.stageFlags = static_cast<VkShaderStageFlags>(visibility);
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.binding = _register;
		binding.pImmutableSamplers = nullptr;

		m_bindings.push_back(binding);
	}


	void InitAsDescriptorRange(DescriptorType type, uint32_t _register, uint32_t count, ShaderVisibility visibility = ShaderVisibility::All)
	{
		InitAsDescriptorTable(1, visibility);
		SetTableRange(0, type, _register, count);
	}


	void InitAsDescriptorTable(uint32_t rangeCount, ShaderVisibility visibility = ShaderVisibility::All)
	{
		assert(m_type == RootParameterType::Invalid);
		assert(m_bindings.empty());

		m_type = RootParameterType::DescriptorTable;
		m_visibility = visibility;

		m_bindings.reserve(rangeCount);
	}


	void SetTableRange(uint32_t rangeIndex, DescriptorType type, uint32_t shaderReg, uint32_t count, uint32_t space = 0)
	{
		assert(m_type == RootParameterType::DescriptorTable);
		assert(rangeIndex < m_bindings.capacity());

		VkDescriptorSetLayoutBinding binding;
		binding.stageFlags = static_cast<VkShaderStageFlags>(m_visibility);
		binding.descriptorCount = count;
		binding.descriptorType = DescriptorTypeToVulkan(type);
		binding.binding = m_currentBinding;
		binding.pImmutableSamplers = nullptr;

		m_bindings.push_back(binding);

		++m_currentBinding;

		DescriptorRange range;
		range.type = type;
		range.numDescriptors = count;
		m_ranges.emplace_back(range);
	}


	VkDescriptorSetLayout GetLayout() { return m_descriptorSetLayout; }
	constexpr VkDescriptorSetLayout GetLayout() const { return m_descriptorSetLayout; }


	uint32_t GetNumRanges() const { return static_cast<uint32_t>(m_ranges.size()); }
	const DescriptorRange& GetRangeDesc(uint32_t index) const { return m_ranges[index]; }


	RootParameterType GetType() const { return m_type; }


protected:
	RootParameterType	m_type{ RootParameterType::Invalid };
	ShaderVisibility	m_visibility;

	VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };

	std::vector<DescriptorRange> m_ranges;

	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
	uint32_t m_currentBinding{ 0 };
};


class RootSignature
{
	friend class DynamicDescriptorPool;

public:
	RootSignature(uint32_t numRootParams = 0, uint32_t numStaticSamplers = 0) : m_numParameters(numRootParams)
	{
		Reset(numRootParams, numStaticSamplers);
	}

	~RootSignature() = default;

	static void DestroyAll();
	void Destroy();

	void Reset(uint32_t numRootParams, uint32_t numStaticSamplers = 0)
	{
		if (numRootParams > 0)
		{
			m_paramArray.reset(new RootParameter[numRootParams]);
		}
		else
		{
			m_paramArray = nullptr;
		}
		m_numParameters = numRootParams;

		if (numStaticSamplers > 0)
		{
			m_samplerArray.reset(new SamplerInfo[numStaticSamplers]);
		}
		else
		{
			m_samplerArray = nullptr;
		}
		m_numSamplers = numStaticSamplers;
		m_numInitializedStaticSamplers = 0;
	}

	RootParameter& operator[] (size_t entryIndex)
	{
		assert(entryIndex < m_numParameters);
		return m_paramArray.get()[entryIndex];
	}

	const RootParameter& operator[] (size_t entryIndex) const
	{
		assert(entryIndex < m_numParameters);
		return m_paramArray.get()[entryIndex];
	}

	void InitStaticSampler(uint32_t _register, const SamplerStateDesc& nonStaticSamplerDesc,
		ShaderVisibility visibility = ShaderVisibility::All);

	void Finalize(const std::string& name, RootSignatureFlags flags = RootSignatureFlags::None);

	VkPipelineLayout GetLayout() const { return m_layout; }

protected:
	bool m_finalized{ false };
	uint32_t m_numParameters{ 0 };
	uint32_t m_numSamplers{ 0 };
	uint32_t m_numInitializedStaticSamplers{ 0 };
	std::unique_ptr<RootParameter[]> m_paramArray;

	struct SamplerInfo
	{
		uint32_t _register;
		ShaderVisibility visibility;
		VkSamplerCreateInfo createInfo;
	};
	std::unique_ptr<SamplerInfo[]> m_samplerArray;
	uint32_t m_staticSamplerSetIndex{ 0xFFFFFFFF };

	std::vector<VkSampler> m_samplers;

	VkDescriptorSetLayout m_samplerLayout{ VK_NULL_HANDLE };
	VkDescriptorSet m_staticSamplerSet{ VK_NULL_HANDLE };

	VkPipelineLayout m_layout{ VK_NULL_HANDLE };
};

} // namespace Kodiak