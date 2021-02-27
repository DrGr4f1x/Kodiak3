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

#include "ResourceSetVk.h"

#include "Graphics\ColorBuffer.h"
#include "Graphics\DepthBuffer.h"
#include "Graphics\DescriptorHeap.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\GraphicsDevice.h"
#include "Graphics\RootSignature.h"
#include "Graphics\Texture.h"


using namespace std;
using namespace Kodiak;


void ResourceSet::Init(const RootSignature* rootSig)
{
	m_rootSig = rootSig;

	for (uint32_t i = 0; i < m_dynamicOffsets.size(); ++i)
	{
		m_dynamicOffsets[i] = 0;
	}

	int rootIndex = 0;
	for (uint32_t i = 0; i < m_rootSig->GetNumParameters(); ++i)
	{
		auto& resourceTable = m_resourceTables[i];

		const auto& rootParameter = (*m_rootSig)[i];

		if (rootParameter.GetType() == RootParameterType::DynamicRootCBV)
			resourceTable.isDynamicCBV = true;

		uint32_t numDescriptors = (*m_rootSig)[i].GetNumDescriptors();

		if (numDescriptors == 0)
			continue;

		resourceTable.descriptorSet = AllocateDescriptorSet((*m_rootSig)[i].GetLayout());

		resourceTable.writeDescriptorSets.resize(numDescriptors);

		for (uint32_t j = 0; j < numDescriptors; ++j)
		{
			VkWriteDescriptorSet& writeSet = resourceTable.writeDescriptorSets[j];
			writeSet = {};
			writeSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeSet.pNext = nullptr;
		}
		resourceTable.rootIndex = rootIndex;
		++rootIndex;
	}

	m_staticSamplers = m_rootSig->GetStaticSamplers();
	if (m_staticSamplers != VK_NULL_HANDLE)
		m_staticSamplerIndex = rootIndex;
}


void ResourceSet::Finalize()
{
	for (uint32_t i = 0; i < 8; ++i)
	{
		ResourceTable& resourceTable = m_resourceTables[i];

		if (resourceTable.rootIndex == -1)
			break;

		if (resourceTable.writeDescriptorSets.empty())
			continue;

		vector<VkWriteDescriptorSet> liveDescriptors;
		liveDescriptors.reserve(resourceTable.writeDescriptorSets.size());
		for (auto& writeDescriptorSet : resourceTable.writeDescriptorSets)
		{
			if (writeDescriptorSet.descriptorCount > 0)
			{
				liveDescriptors.push_back(writeDescriptorSet);
			}
		}

		vkUpdateDescriptorSets(
			GetDevice(), 
			(uint32_t)liveDescriptors.size(), 
			liveDescriptors.data(),
			0, 
			nullptr);
	}
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const ColorBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_resourceTables[rootIndex].writeDescriptorSets[paramIndex];

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeSet.dstSet = m_resourceTables[rootIndex].descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = buffer.GetSRVImageInfoPtr();
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const DepthBuffer& buffer, bool depthSrv)
{
	VkWriteDescriptorSet& writeSet = m_resourceTables[rootIndex].writeDescriptorSets[paramIndex];

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeSet.dstSet = m_resourceTables[rootIndex].descriptorSet;
	writeSet.dstBinding = paramIndex;

	if (depthSrv)
	{
		writeSet.pImageInfo = buffer.GetDepthImageInfoPtr();
	}
	else
	{
		writeSet.pImageInfo = buffer.GetStencilImageInfoPtr();
	}
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const StructuredBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_resourceTables[rootIndex].writeDescriptorSets[paramIndex];

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeSet.dstSet = m_resourceTables[rootIndex].descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pBufferInfo = buffer.GetBufferInfoPtr();
}


void ResourceSet::SetSRV(int rootIndex, int paramIndex, const Texture& texture)
{
	VkWriteDescriptorSet& writeSet = m_resourceTables[rootIndex].writeDescriptorSets[paramIndex];

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeSet.dstSet = m_resourceTables[rootIndex].descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = texture.GetImageInfoPtr();
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const ColorBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_resourceTables[rootIndex].writeDescriptorSets[paramIndex];

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	writeSet.dstSet = m_resourceTables[rootIndex].descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = buffer.GetUAVImageInfoPtr();
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const DepthBuffer& buffer)
{
	assert_msg(false, "Depth UAVs not yet supported");
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const StructuredBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_resourceTables[rootIndex].writeDescriptorSets[paramIndex];

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeSet.dstSet = m_resourceTables[rootIndex].descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pBufferInfo = buffer.GetBufferInfoPtr();
}


void ResourceSet::SetUAV(int rootIndex, int paramIndex, const Texture& texture)
{
	assert_msg(false, "Texture UAVs not yet supported");
}


void ResourceSet::SetCBV(int rootIndex, int paramIndex, const ConstantBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_resourceTables[rootIndex].writeDescriptorSets[paramIndex];

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = m_resourceTables[rootIndex].isDynamicCBV ? VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeSet.dstSet = m_resourceTables[rootIndex].descriptorSet;
	writeSet.dstBinding = paramIndex;
	writeSet.pBufferInfo = buffer.GetBufferInfoPtr();
}


void ResourceSet::SetDynamicOffset(int rootIndex, uint32_t offset)
{
	assert(m_resourceTables[rootIndex].isDynamicCBV);

	m_dynamicOffsets[rootIndex] = offset;
}