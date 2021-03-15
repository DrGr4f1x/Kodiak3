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

#include "DescriptorSetVk.h"

#include "Graphics\ColorBuffer.h"
#include "Graphics\DepthBuffer.h"
#include "Graphics\DescriptorHeap.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\GraphicsDevice.h"
#include "Graphics\RootSignature.h"
#include "Graphics\Texture.h"


using namespace Kodiak;
using namespace std;


void DescriptorSet::Init(const RootSignature& rootSig, int rootParam)
{
	const auto& rootParameter = rootSig[rootParam];

	if (rootParameter.GetType() == RootParameterType::DynamicRootCBV)
		m_isDynamicCBV = true;

	const uint32_t numDescriptors = rootParameter.GetNumDescriptors();
	assert(numDescriptors <= MaxDescriptors);

	if (numDescriptors == 0)
		return;

	m_descriptorSet = AllocateDescriptorSet(rootSig[rootParam].GetLayout());

	for (uint32_t j = 0; j < numDescriptors; ++j)
	{
		VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[j];
		writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeSet.pNext = nullptr;
	}

	m_bIsInitialized = true;
}


void DescriptorSet::SetSRV(int paramIndex, const ColorBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pImageInfo == buffer.GetSRVImageInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeSet.dstSet = m_descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = buffer.GetSRVImageInfoPtr();

	m_dirtyBits |= (1 << paramIndex);
}


void DescriptorSet::SetSRV(int paramIndex, const DepthBuffer& buffer, bool depthSrv)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	auto imageInfo = depthSrv ? buffer.GetDepthImageInfoPtr() : buffer.GetStencilImageInfoPtr();
	if (writeSet.pImageInfo == imageInfo)
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeSet.dstSet = m_descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = imageInfo;
	
	m_dirtyBits |= (1 << paramIndex);
}


void DescriptorSet::SetSRV(int paramIndex, const StructuredBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pBufferInfo == buffer.GetBufferInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeSet.dstSet = m_descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pBufferInfo = buffer.GetBufferInfoPtr();

	m_dirtyBits |= (1 << paramIndex);
}


void DescriptorSet::SetSRV(int paramIndex, const Texture& texture)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pImageInfo == texture.GetImageInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeSet.dstSet = m_descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = texture.GetImageInfoPtr();

	m_dirtyBits |= (1 << paramIndex);
}


void DescriptorSet::SetUAV(int paramIndex, const ColorBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pImageInfo == buffer.GetUAVImageInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	writeSet.dstSet = m_descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = buffer.GetUAVImageInfoPtr();

	m_dirtyBits |= (1 << paramIndex);
}


void DescriptorSet::SetUAV(int paramIndex, const DepthBuffer& buffer)
{
	assert_msg(false, "Depth UAVs not yet supported");
}


void DescriptorSet::SetUAV(int paramIndex, const StructuredBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pBufferInfo == buffer.GetBufferInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeSet.dstSet = m_descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pBufferInfo = buffer.GetBufferInfoPtr();

	m_dirtyBits |= (1 << paramIndex);
}


void DescriptorSet::SetUAV(int paramIndex, const Texture& texture)
{
	assert_msg(false, "Texture UAVs not yet supported");
}


void DescriptorSet::SetCBV(int paramIndex, const ConstantBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pBufferInfo == buffer.GetBufferInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = m_isDynamicCBV ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeSet.dstSet = m_descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pBufferInfo = buffer.GetBufferInfoPtr();

	m_dirtyBits |= (1 << paramIndex);
}


void DescriptorSet::SetDynamicOffset(uint32_t offset)
{
	assert(m_isDynamicCBV);

	m_dynamicOffset = offset;
}


void DescriptorSet::Update()
{
	if (!IsDirty() || m_writeDescriptorSets.empty())
		return;

	array<VkWriteDescriptorSet, MaxDescriptors> liveDescriptors;

	unsigned long setBit{ 0 };
	uint32_t index{ 0 };
	while (_BitScanForward(&setBit, m_dirtyBits))
	{
		liveDescriptors[index++] = m_writeDescriptorSets[setBit];
		m_dirtyBits &= ~(1 << setBit);
	}

	assert(m_dirtyBits == 0);

	vkUpdateDescriptorSets(
		GetDevice(),
		(uint32_t)index,
		liveDescriptors.data(),
		0,
		nullptr);
}