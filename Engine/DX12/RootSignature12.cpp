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

#include "Stdafx.h"

#include "RootSignature12.h"

#include "Hash.h"

#include "GraphicsDevice12.h"


using namespace Kodiak;
using namespace std;


namespace
{
map<size_t, Microsoft::WRL::ComPtr<ID3D12RootSignature>> s_rootSignatureHashMap;
} // anonymous namespace


void RootSignature::DestroyAll()
{
	s_rootSignatureHashMap.clear();
}


void RootSignature::InitStaticSampler(
	uint32_t _register,
	const D3D12_SAMPLER_DESC& nonStaticSamplerDesc,
	ShaderVisibility visibility)
{
	assert(m_numInitializedStaticSamplers < m_numSamplers);
	D3D12_STATIC_SAMPLER_DESC& staticSamplerDesc = m_samplerArray[m_numInitializedStaticSamplers++];

	staticSamplerDesc.Filter = nonStaticSamplerDesc.Filter;
	staticSamplerDesc.AddressU = nonStaticSamplerDesc.AddressU;
	staticSamplerDesc.AddressV = nonStaticSamplerDesc.AddressV;
	staticSamplerDesc.AddressW = nonStaticSamplerDesc.AddressW;
	staticSamplerDesc.MipLODBias = nonStaticSamplerDesc.MipLODBias;
	staticSamplerDesc.MaxAnisotropy = nonStaticSamplerDesc.MaxAnisotropy;
	staticSamplerDesc.ComparisonFunc = nonStaticSamplerDesc.ComparisonFunc;
	staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
	staticSamplerDesc.MinLOD = nonStaticSamplerDesc.MinLOD;
	staticSamplerDesc.MaxLOD = nonStaticSamplerDesc.MaxLOD;
	staticSamplerDesc.ShaderRegister = _register;
	staticSamplerDesc.RegisterSpace = 0;
	staticSamplerDesc.ShaderVisibility = static_cast<D3D12_SHADER_VISIBILITY>(visibility);

	if (staticSamplerDesc.AddressU == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		staticSamplerDesc.AddressV == D3D12_TEXTURE_ADDRESS_MODE_BORDER ||
		staticSamplerDesc.AddressW == D3D12_TEXTURE_ADDRESS_MODE_BORDER)
	{
		warn_once_if_not(
			// Transparent Black
			nonStaticSamplerDesc.BorderColor[0] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[1] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[2] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[3] == 0.0f ||
			// Opaque Black
			nonStaticSamplerDesc.BorderColor[0] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[1] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[2] == 0.0f &&
			nonStaticSamplerDesc.BorderColor[3] == 1.0f ||
			// Opaque White
			nonStaticSamplerDesc.BorderColor[0] == 1.0f &&
			nonStaticSamplerDesc.BorderColor[1] == 1.0f &&
			nonStaticSamplerDesc.BorderColor[2] == 1.0f &&
			nonStaticSamplerDesc.BorderColor[3] == 1.0f,
			"Sampler border color does not match static sampler limitations");

		if (nonStaticSamplerDesc.BorderColor[3] == 1.0f)
		{
			if (nonStaticSamplerDesc.BorderColor[0] == 1.0f)
			{
				staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_WHITE;
			}
			else
			{
				staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
			}
		}
		else
		{
			staticSamplerDesc.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
		}
	}
}


void RootSignature::Finalize(const string& name, RootSignatureFlags flags)
{
	if (m_finalized)
	{
		return;
	}

	assert(m_numInitializedStaticSamplers == m_numSamplers);

	D3D12_ROOT_SIGNATURE_DESC rootDesc;
	rootDesc.NumParameters = m_numParameters;
	rootDesc.pParameters = (const D3D12_ROOT_PARAMETER*)m_paramArray.get();
	rootDesc.NumStaticSamplers = m_numSamplers;
	rootDesc.pStaticSamplers = (const D3D12_STATIC_SAMPLER_DESC*)m_samplerArray.get();
	rootDesc.Flags = static_cast<D3D12_ROOT_SIGNATURE_FLAGS>(flags);

	m_descriptorTableBitMap = 0;
	m_samplerTableBitMap = 0;

	size_t hashCode = Utility::HashState(&rootDesc.Flags);
	hashCode = Utility::HashState(rootDesc.pStaticSamplers, m_numSamplers, hashCode);

	for (uint32_t param = 0; param < m_numParameters; ++param)
	{
		const D3D12_ROOT_PARAMETER& rootParam = rootDesc.pParameters[param];
		m_descriptorTableSize[param] = 0;

		if (rootParam.ParameterType == D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE)
		{
			assert(rootParam.DescriptorTable.pDescriptorRanges != nullptr);

			hashCode = Utility::HashState(rootParam.DescriptorTable.pDescriptorRanges,
				rootParam.DescriptorTable.NumDescriptorRanges, hashCode);

			// We keep track of sampler descriptor tables separately from CBV_SRV_UAV descriptor tables
			if (rootParam.DescriptorTable.pDescriptorRanges->RangeType == D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER)
			{
				m_samplerTableBitMap |= (1 << param);
			}
			else
			{
				m_descriptorTableBitMap |= (1 << param);
			}

			for (uint32_t tableRange = 0; tableRange < rootParam.DescriptorTable.NumDescriptorRanges; ++tableRange)
			{
				m_descriptorTableSize[param] += rootParam.DescriptorTable.pDescriptorRanges[tableRange].NumDescriptors;
			}
		}
		else
		{
			hashCode = Utility::HashState(&rootParam, 1, hashCode);
		}
	}

	ID3D12RootSignature** RSRef = nullptr;
	bool firstCompile = false;
	{
		static mutex s_hashMapMutex;
		lock_guard<mutex> CS(s_hashMapMutex);

		auto iter = s_rootSignatureHashMap.find(hashCode);

		// Reserve space so the next inquiry will find that someone got here first.
		if (iter == s_rootSignatureHashMap.end())
		{
			RSRef = s_rootSignatureHashMap[hashCode].GetAddressOf();
			firstCompile = true;
		}
		else
		{
			RSRef = iter->second.GetAddressOf();
		}
	}

	if (firstCompile)
	{
		Microsoft::WRL::ComPtr<ID3DBlob> pOutBlob, pErrorBlob;

		assert_succeeded(D3D12SerializeRootSignature(&rootDesc, D3D_ROOT_SIGNATURE_VERSION_1,
			pOutBlob.GetAddressOf(), pErrorBlob.GetAddressOf()));

		assert_succeeded(GetDevice()->CreateRootSignature(1, pOutBlob->GetBufferPointer(), pOutBlob->GetBufferSize(),
			MY_IID_PPV_ARGS(&m_signature)));

		m_signature->SetName(MakeWStr(name).c_str());

		s_rootSignatureHashMap[hashCode].Attach(m_signature);
		assert(*RSRef == m_signature);
	}
	else
	{
		while (*RSRef == nullptr)
		{
			this_thread::yield();
		}
		m_signature = *RSRef;
	}

	m_finalized = TRUE;
}