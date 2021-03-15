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

#include "DescriptorSet12.h"

#include "Graphics\ColorBuffer.h"
#include "Graphics\DepthBuffer.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\GraphicsDevice.h"
#include "Graphics\RootSignature.h"
#include "Graphics\Texture.h"

#include "DescriptorHeap12.h"


using namespace Kodiak;
using namespace std;


void DescriptorSet::Init(const RootSignature& rootSig, int rootParam)
{
	const auto& rootParameter = rootSig[rootParam];

	const uint32_t numDescriptors = rootParameter.GetNumDescriptors();
	assert(numDescriptors <= MaxDescriptors);

	if (numDescriptors == 0)
		return;

	for (uint32_t j = 0; j < numDescriptors; ++j)
	{
		m_descriptors[j].ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
	}
	m_gpuDescriptor.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;

	m_bIsRootCBV = rootParameter.IsRootCBV();

	m_bIsInitialized = true;
}


void DescriptorSet::SetSRV(int paramIndex, const ColorBuffer& buffer)
{
	SetDescriptor(paramIndex, buffer.GetSRV());
}


void DescriptorSet::SetSRV(int paramIndex, const DepthBuffer& buffer, bool depthSrv)
{
	SetDescriptor(paramIndex, depthSrv ? buffer.GetDepthSRV() : buffer.GetStencilSRV());
}


void DescriptorSet::SetSRV(int paramIndex, const StructuredBuffer& buffer)
{
	SetDescriptor(paramIndex, buffer.GetSRV());
}


void DescriptorSet::SetSRV(int paramIndex, const Texture& texture)
{
	SetDescriptor(paramIndex, texture.GetSRV());
}


void DescriptorSet::SetUAV(int paramIndex, const ColorBuffer& buffer)
{
	SetDescriptor(paramIndex, buffer.GetUAV());
}


void DescriptorSet::SetUAV(int paramIndex, const DepthBuffer& buffer)
{
	assert_msg(false, "Depth UAVs not yet supported");
}


void DescriptorSet::SetUAV(int paramIndex, const StructuredBuffer& buffer)
{
	SetDescriptor(paramIndex, buffer.GetUAV());
}


void DescriptorSet::SetUAV(int paramIndex, const Texture& texture)
{
	assert_msg(false, "Texture UAVs not yet supported");
}


void DescriptorSet::SetCBV(int paramIndex, const ConstantBuffer& buffer)
{
	if (m_bIsRootCBV)
	{
		m_gpuAddress = buffer.GetGpuAddress();
	}
	else
	{
		SetDescriptor(paramIndex, buffer.GetCBV());
	}
}


void DescriptorSet::SetDynamicOffset(uint32_t offset)
{
	assert(m_gpuAddress != D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN);

	m_dynamicOffset = offset;
}


void DescriptorSet::Update()
{
	if (!IsDirty() || m_descriptors.empty())
		return;

	auto device = GetDevice();

	const uint32_t numDescriptors = __popcnt(m_dirtyBits);

	D3D12_DESCRIPTOR_HEAP_TYPE heapType{ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV };

	if (m_bIsSamplerTable)
	{
		heapType = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	}

	DescriptorHandle descHandle = AllocateUserDescriptor(heapType, numDescriptors);
	uint32_t descriptorSize = device->GetDescriptorHandleIncrementSize(heapType);

	m_gpuDescriptor = descHandle.GetGpuHandle();

	unsigned long setBit{ 0 };
	uint32_t index{ 0 };
	while (_BitScanForward(&setBit, m_dirtyBits))
	{
		DescriptorHandle offsetHandle = descHandle + index * descriptorSize;
		m_dirtyBits &= ~(1 << setBit);

		device->CopyDescriptorsSimple(1, offsetHandle.GetCpuHandle(), m_descriptors[index], heapType);

		++index;
	}

	assert(m_dirtyBits == 0);
}


void DescriptorSet::SetDescriptor(int paramIndex, D3D12_CPU_DESCRIPTOR_HANDLE descriptor)
{
	if (m_descriptors[paramIndex].ptr != descriptor.ptr)
	{
		m_descriptors[paramIndex] = descriptor;
		m_dirtyBits |= (1 << paramIndex);
	}
}