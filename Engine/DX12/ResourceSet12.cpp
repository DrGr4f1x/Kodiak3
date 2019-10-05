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

#include "ResourceSet.h"

#include "ColorBuffer.h"
#include "DepthBuffer.h"
#include "GpuBuffer.h"
#include "GraphicsDevice.h"
#include "RootSignature.h"
#include "Texture.h"

#include "DescriptorHeap12.h"


using namespace std;
using namespace Kodiak;


void ResourceSet::Init(const RootSignature* rootSig)
{
	m_rootSig = rootSig;

	for (uint32_t i = 0; i < m_rootSig->GetNumParameters(); ++i)
	{
		uint32_t numDescriptors = (*m_rootSig)[i].GetNumDescriptors();
		m_resourceTables[i].descriptors.resize(numDescriptors);
		for (uint32_t j = 0; j < numDescriptors; ++j)
		{
			m_resourceTables[i].descriptors[j].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		}
		m_resourceTables[i].isSamplerTable = false;
	}
}


void ResourceSet::Finalize()
{
	assert(m_rootSig != nullptr);

	auto device = g_graphicsDevice->GetDevice();

	for (uint32_t i = 0; i < 8; ++i)
	{
		if (m_resourceTables[i].descriptors.empty())
			break;

		uint32_t numDescriptors = static_cast<uint32_t>(m_resourceTables[i].descriptors.size());
		D3D12_DESCRIPTOR_HEAP_TYPE heapType{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

		if (m_resourceTables[i].isSamplerTable)
		{
			heapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;	
		}

		DescriptorHandle descHandle = AllocateUserDescriptor(heapType, numDescriptors);
		uint32_t descriptorSize = device->GetDescriptorHandleIncrementSize(heapType);

		m_resourceTables[i].gpuDescriptor = descHandle.GetGpuHandle();

		for (uint32_t j = 0; j < numDescriptors; ++j)
		{
			DescriptorHandle offsetHandle = descHandle + j * descriptorSize;
			device->CopyDescriptorsSimple(1, offsetHandle.GetCpuHandle(), m_resourceTables[i].descriptors[j], heapType);
		}
	}

	m_rootSig = nullptr;
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const ColorBuffer& buffer)
{
	m_resourceTables[rootIndex].descriptors[paramIndex] = buffer.GetSRV().GetHandle();
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const DepthBuffer& buffer, bool depthSrv)
{
	if (depthSrv)
	{
		m_resourceTables[rootIndex].descriptors[paramIndex] = buffer.GetDepthSRV().GetHandle();
	}
	else
	{
		m_resourceTables[rootIndex].descriptors[paramIndex] = buffer.GetStencilSRV().GetHandle();
	}
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const StructuredBuffer& buffer)
{
	m_resourceTables[rootIndex].descriptors[paramIndex] = buffer.GetSRV().GetHandle();
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const Texture& texture)
{
	m_resourceTables[rootIndex].descriptors[paramIndex] = texture.GetSRV().GetHandle();
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const ColorBuffer& buffer)
{
	m_resourceTables[rootIndex].descriptors[paramIndex] = buffer.GetUAV().GetHandle();
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const DepthBuffer& buffer)
{
	assert_msg(false, "Depth UAVs not yet supported");
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const StructuredBuffer& buffer)
{
	m_resourceTables[rootIndex].descriptors[paramIndex] = buffer.GetUAV().GetHandle();
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const Texture& texture)
{
	assert_msg(false, "Texture UAVs not yet supported");
}


void ResourceSet::SetCBV(int rootIndex, int paramIndex, const ConstantBuffer& buffer)
{
	m_resourceTables[rootIndex].descriptors[paramIndex] = buffer.GetCBV().GetHandle();
}