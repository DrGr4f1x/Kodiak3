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

class RootParameter
{
	friend class RootSignature;

public:
	RootParameter()
	{}

	~RootParameter();

	void InitAsConstantBuffer(uint32_t _register, ShaderVisibility visibility = ShaderVisibility::All)
	{
		m_visibility = visibility;

		VkDescriptorSetLayoutBinding binding;
		binding.stageFlags = static_cast<VkShaderStageFlags>(visibility);
		binding.descriptorCount = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		binding.binding = _register;
		binding.pImmutableSamplers = nullptr;

		m_bindings.push_back(binding);
	}

	VkDescriptorSetLayout GetLayout() { return m_descriptorSetLayout; }
	constexpr VkDescriptorSetLayout GetLayout() const { return m_descriptorSetLayout; }

protected:
	ShaderVisibility	m_visibility;

	VkDescriptorSetLayoutCreateInfo m_layoutInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO };
	VkDescriptorSetLayout m_descriptorSetLayout{ VK_NULL_HANDLE };

	std::vector<VkDescriptorSetLayoutBinding> m_bindings;
};


class RootSignature
{
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

		// TODO
#if 0
		if (numStaticSamplers > 0)
		{
			m_samplerArray.reset(new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers]);
		}
		else
		{
			m_samplerArray = nullptr;
		}
		m_numSamplers = numStaticSamplers;
		m_numInitializedStaticSamplers = 0;
#endif
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

	void Finalize(const std::string& name, RootSignatureFlags flags = RootSignatureFlags::None);

	VkPipelineLayout GetLayout() const { return m_layout; }

protected:
	bool m_finalized{ false };
	uint32_t m_numParameters{ 0 };
	std::unique_ptr<RootParameter[]> m_paramArray;
	VkPipelineLayout m_layout{ VK_NULL_HANDLE };
};

} // namespace Kodiak