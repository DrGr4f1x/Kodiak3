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

namespace Kodiak
{

// Forward declarations
struct SamplerStateDesc;


class RootParameter
{
	friend class RootSignature;
public:

	RootParameter()
	{
		m_rootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}

	~RootParameter()
	{
		Clear();
	}


	void Clear()
	{
		if (m_rootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			delete[] m_rootParam.DescriptorTable.pDescriptorRanges;
		}

		m_rootParam.ParameterType = (D3D12_ROOT_PARAMETER_TYPE)0xFFFFFFFF;
	}


	void InitAsConstants(uint32_t _register, uint32_t numDwords, ShaderVisibility visibility = ShaderVisibility::All)
	{
		m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		m_rootParam.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(visibility);
		m_rootParam.Constants.Num32BitValues = numDwords;
		m_rootParam.Constants.ShaderRegister = _register;
		m_rootParam.Constants.RegisterSpace = 0;
	}


	void InitAsConstantBuffer(uint32_t _register, ShaderVisibility visibility = ShaderVisibility::All)
	{
		m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		m_rootParam.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(visibility);
		m_rootParam.Descriptor.ShaderRegister = _register;
		m_rootParam.Descriptor.RegisterSpace = 0;
	}


	void InitAsDynamicConstantBuffer(uint32_t _register, ShaderVisibility visibility = ShaderVisibility::All)
	{
		InitAsConstantBuffer(_register, visibility);
	}


	void InitAsDescriptorRange(DescriptorType type, uint32_t _register, uint32_t count, ShaderVisibility visibility = ShaderVisibility::All)
	{
		InitAsDescriptorTable(1, visibility);
		SetTableRange(0, type, _register, count);
	}


	void InitAsDescriptorTable(uint32_t rangeCount, ShaderVisibility visibility = ShaderVisibility::All)
	{
		m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		m_rootParam.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(visibility);
		m_rootParam.DescriptorTable.NumDescriptorRanges = rangeCount;
		m_rootParam.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[rangeCount];
	}


	void SetTableRange(uint32_t rangeIndex, DescriptorType type, uint32_t _register, uint32_t count, uint32_t space = 0)
	{
		D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(m_rootParam.DescriptorTable.pDescriptorRanges + rangeIndex);
		range->RangeType = DescriptorTypeToDX12(type);
		range->NumDescriptors = count;
		range->BaseShaderRegister = _register;
		range->RegisterSpace = space;
		range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	//const D3D12_ROOT_PARAMETER& operator()() const { return m_rootParam; }

	uint32_t GetNumDescriptors() const;

protected:
	D3D12_ROOT_PARAMETER m_rootParam;
};


// Maximum 64 DWORDS divied up amongst all root parameters.
// Root constants = 1 DWORD * NumConstants
// Root descriptor (CBV, SRV, or UAV) = 2 DWORDs each
// Descriptor table pointer = 1 DWORD
// Static samplers = 0 DWORDS (compiled into shader)
class RootSignature
{
	friend class DynamicDescriptorHeap;

public:

	RootSignature(uint32_t numRootParams = 0, uint32_t numStaticSamplers = 0) : m_numParameters(numRootParams)
	{
		Reset(numRootParams, numStaticSamplers);
	}

	~RootSignature() = default;

	static void DestroyAll();
	void Destroy() {}


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
			m_samplerArray.reset(new D3D12_STATIC_SAMPLER_DESC[numStaticSamplers]);
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

	uint32_t GetNumParameters() const { return m_numParameters; }

	void InitStaticSampler(uint32_t _register, const SamplerStateDesc& nonStaticSamplerDesc,
		ShaderVisibility visibility = ShaderVisibility::All);

	void Finalize(const std::string& name, RootSignatureFlags flags = RootSignatureFlags::None);

	ID3D12RootSignature* GetSignature() const { return m_signature; }

protected:
	bool m_finalized{ false };
	uint32_t m_numParameters{ 0 };
	uint32_t m_numSamplers{ 0 };
	uint32_t m_numInitializedStaticSamplers{ 0 };
	uint32_t m_descriptorTableBitMap{ 0 };		// One bit is set for root parameters that are non-sampler descriptor tables
	uint32_t m_samplerTableBitMap{ 0 };			// One bit is set for root parameters that are sampler descriptor tables
	uint32_t m_descriptorTableSize[16];		// Non-sampler descriptor tables need to know their descriptor count
	std::unique_ptr<RootParameter[]> m_paramArray;
	std::unique_ptr<D3D12_STATIC_SAMPLER_DESC[]> m_samplerArray;
	ID3D12RootSignature* m_signature{ nullptr };
};

} // namespace Kodiak