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

	void InitAsConstants(uint32_t _register, uint32_t numDwords, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
	{
		m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
		m_rootParam.ShaderVisibility = visibility;
		m_rootParam.Constants.Num32BitValues = numDwords;
		m_rootParam.Constants.ShaderRegister = _register;
		m_rootParam.Constants.RegisterSpace = 0;
	}

	void InitAsConstantBuffer(uint32_t _register, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
	{
		m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
		m_rootParam.ShaderVisibility = visibility;
		m_rootParam.Descriptor.ShaderRegister = _register;
		m_rootParam.Descriptor.RegisterSpace = 0;
	}

	void InitAsBufferSRV(uint32_t _register, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
	{
		m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
		m_rootParam.ShaderVisibility = visibility;
		m_rootParam.Descriptor.ShaderRegister = _register;
		m_rootParam.Descriptor.RegisterSpace = 0;
	}

	void InitAsBufferUAV(uint32_t _register, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
	{
		m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_UAV;
		m_rootParam.ShaderVisibility = visibility;
		m_rootParam.Descriptor.ShaderRegister = _register;
		m_rootParam.Descriptor.RegisterSpace = 0;
	}

	void InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t _register, uint32_t count, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
	{
		InitAsDescriptorTable(1, visibility);
		SetTableRange(0, type, _register, count);
	}

	void InitAsDescriptorTable(uint32_t rangeCount, D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL)
	{
		m_rootParam.ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		m_rootParam.ShaderVisibility = visibility;
		m_rootParam.DescriptorTable.NumDescriptorRanges = rangeCount;
		m_rootParam.DescriptorTable.pDescriptorRanges = new D3D12_DESCRIPTOR_RANGE[rangeCount];
	}

	void SetTableRange(uint32_t rangeIndex, D3D12_DESCRIPTOR_RANGE_TYPE type, uint32_t _register, uint32_t count, uint32_t space = 0)
	{
		D3D12_DESCRIPTOR_RANGE* range = const_cast<D3D12_DESCRIPTOR_RANGE*>(m_rootParam.DescriptorTable.pDescriptorRanges + rangeIndex);
		range->RangeType = type;
		range->NumDescriptors = count;
		range->BaseShaderRegister = _register;
		range->RegisterSpace = space;
		range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
	}

	const D3D12_ROOT_PARAMETER& operator()() const { return m_rootParam; }

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

	void InitStaticSampler(uint32_t _register, const D3D12_SAMPLER_DESC& nonStaticSamplerDesc,
		D3D12_SHADER_VISIBILITY visibility = D3D12_SHADER_VISIBILITY_ALL);

	void Finalize(const std::string& name, D3D12_ROOT_SIGNATURE_FLAGS flags = D3D12_ROOT_SIGNATURE_FLAG_NONE);

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