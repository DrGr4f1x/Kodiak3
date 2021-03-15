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

#include "DynamicDescriptorPoolVk.h"

#include "Graphics\ColorBuffer.h"
#include "Graphics\CommandContext.h"
#include "Graphics\DepthBuffer.h"
#include "Graphics\DescriptorHeap.h"
#include "Graphics\GpuBuffer.h"
#include "Graphics\GraphicsDevice.h"
#include "Graphics\RootSignature.h"
#include "Graphics\Texture.h"


using namespace Kodiak;
using namespace std;


DynamicDescriptorSet::DynamicDescriptorSet()
{
	Invalidate();
}


void DynamicDescriptorSet::Init(const RootSignature& rootSig, int rootParam)
{
	const auto& rootParameter = rootSig[rootParam];

	if (rootParameter.GetType() != RootParameterType::DescriptorTable)
		return;

	const uint32_t numDescriptors = rootParameter.GetNumDescriptors();
	assert(numDescriptors <= MaxDescriptors);

	if (numDescriptors == 0)
		return;

	m_layout = rootParameter.GetLayout();

	m_bIsInitialized = true;
}


void DynamicDescriptorSet::Invalidate()
{
	m_layout = VK_NULL_HANDLE;
	for (uint32_t j = 0; j < MaxDescriptors; ++j)
	{
		VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[j];
		writeSet = VkWriteDescriptorSet{ VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET };
	}

	m_bIsInitialized = 0;
	m_dirtyBits = 0;

	m_allocation.Reset();
}


void DynamicDescriptorSet::SetSRV(int paramIndex, const ColorBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pImageInfo == buffer.GetSRVImageInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = buffer.GetSRVImageInfoPtr();

	m_allocation.numSampledImages++;

	m_dirtyBits |= (1 << paramIndex);
}


void DynamicDescriptorSet::SetSRV(int paramIndex, const DepthBuffer& buffer, bool depthSrv)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	auto imageInfo = depthSrv ? buffer.GetDepthImageInfoPtr() : buffer.GetStencilImageInfoPtr();
	if (writeSet.pImageInfo == imageInfo)
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = imageInfo;

	m_allocation.numSampledImages++;

	m_dirtyBits |= (1 << paramIndex);
}


void DynamicDescriptorSet::SetSRV(int paramIndex, const StructuredBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pBufferInfo == buffer.GetBufferInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeSet.dstBinding = paramIndex;
	writeSet.pBufferInfo = buffer.GetBufferInfoPtr();

	m_allocation.numStorageBuffers++;

	m_dirtyBits |= (1 << paramIndex);
}


void DynamicDescriptorSet::SetSRV(int paramIndex, const Texture& texture)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pImageInfo == texture.GetImageInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = texture.GetImageInfoPtr();

	m_allocation.numSampledImages++;

	m_dirtyBits |= (1 << paramIndex);
}


void DynamicDescriptorSet::SetUAV(int paramIndex, const ColorBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pImageInfo == buffer.GetUAVImageInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	writeSet.dstBinding = paramIndex;
	writeSet.pImageInfo = buffer.GetUAVImageInfoPtr();

	m_allocation.numStorageImages++;

	m_dirtyBits |= (1 << paramIndex);
}


void DynamicDescriptorSet::SetUAV(int paramIndex, const DepthBuffer& buffer)
{
	assert_msg(false, "Depth UAVs not yet supported");
}


void DynamicDescriptorSet::SetUAV(int paramIndex, const StructuredBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pBufferInfo == buffer.GetBufferInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeSet.dstBinding = paramIndex;
	writeSet.pBufferInfo = buffer.GetBufferInfoPtr();

	m_allocation.numStorageBuffers++;

	m_dirtyBits |= (1 << paramIndex);
}


void DynamicDescriptorSet::SetUAV(int paramIndex, const Texture& texture)
{
	assert_msg(false, "Texture UAVs not yet supported");
}


void DynamicDescriptorSet::SetCBV(int paramIndex, const ConstantBuffer& buffer)
{
	VkWriteDescriptorSet& writeSet = m_writeDescriptorSets[paramIndex];

	if (writeSet.pBufferInfo == buffer.GetBufferInfoPtr())
		return;

	writeSet.descriptorCount = 1;
	writeSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeSet.dstBinding = paramIndex;
	writeSet.pBufferInfo = buffer.GetBufferInfoPtr();

	m_allocation.numUniformBuffers++;

	m_dirtyBits |= (1 << paramIndex);
}


const DescriptorAllocation DynamicDescriptorPool::s_maxAllocationPerPool
{
	.numSamplers = 256,
	.numCombinedImageSamplers = 1024,
	.numSampledImages = 1024,
	.numStorageImages = 1024,
	.numUniformTexelBuffers = 256,
	.numStorageTexelBuffers = 256,
	.numUniformBuffers = 1024,
	.numStorageBuffers = 1024,
	.numDynamicUniformBuffers = 1024,
	.numDynamicStorageBuffers = 256,
	.numInputAttachments = 64
};
const uint32_t DynamicDescriptorPool::s_maxDescriptorSets = 256;
std::mutex DynamicDescriptorPool::s_mutex;
std::vector<Microsoft::WRL::ComPtr<UVkDescriptorPool>> DynamicDescriptorPool::s_descriptorPools;
std::queue<std::pair<uint64_t, UVkDescriptorPool*>> DynamicDescriptorPool::s_retiredDescriptorPools;
std::queue<UVkDescriptorPool*> DynamicDescriptorPool::s_availableDescriptorPools;


DynamicDescriptorPool::DynamicDescriptorPool(CommandContext& owningContext)
	: m_owningContext{ owningContext }
{}


void DynamicDescriptorPool::ParseGraphicsRootSignature(const RootSignature& rootSig)
{
	for (auto& descriptorSet : m_graphicsDescriptorSets)
	{
		descriptorSet.Invalidate();
	}

	for (uint32_t i = 0; i < rootSig.GetNumParameters(); ++i)
	{
		auto& rootParam = rootSig[i];

		if (rootParam.GetType() != RootParameterType::DescriptorTable)
			continue;

		m_graphicsDescriptorSets[i].Init(rootSig, i);
	}
}


void DynamicDescriptorPool::ParseComputeRootSignature(const RootSignature& rootSig)
{
	for (auto& descriptorSet : m_computeDescriptorSets)
	{
		descriptorSet.Invalidate();
	}

	for (uint32_t i = 0; i < rootSig.GetNumParameters(); ++i)
	{
		auto& rootParam = rootSig[i];

		if (rootParam.GetType() != RootParameterType::DescriptorTable)
			continue;

		m_computeDescriptorSets[i].Init(rootSig, i);
	}
}


void DynamicDescriptorPool::CleanupUsedPools(uint64_t fenceValue)
{
	RetireCurrentPool();
	RetireUsedPools(fenceValue);
}


void DynamicDescriptorPool::SetGraphicsSRV(int rootIndex, int offset, const ColorBuffer& buffer)
{
	m_graphicsDescriptorSets[rootIndex].SetSRV(offset, buffer);
	m_bAnyGraphicsDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetGraphicsSRV(int rootIndex, int offset, const DepthBuffer& buffer, bool depthSrv)
{
	m_graphicsDescriptorSets[rootIndex].SetSRV(offset, buffer, depthSrv);
	m_bAnyGraphicsDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetGraphicsSRV(int rootIndex, int offset, const StructuredBuffer& buffer)
{
	m_graphicsDescriptorSets[rootIndex].SetSRV(offset, buffer);
	m_bAnyGraphicsDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetGraphicsSRV(int rootIndex, int offset, const Texture& texture)
{
	m_graphicsDescriptorSets[rootIndex].SetSRV(offset, texture);
	m_bAnyGraphicsDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetGraphicsUAV(int rootIndex, int offset, const ColorBuffer& buffer)
{
	m_graphicsDescriptorSets[rootIndex].SetUAV(offset, buffer);
	m_bAnyGraphicsDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetGraphicsUAV(int rootIndex, int offset, const DepthBuffer& buffer)
{
	m_graphicsDescriptorSets[rootIndex].SetUAV(offset, buffer);
	m_bAnyGraphicsDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetGraphicsUAV(int rootIndex, int offset, const StructuredBuffer& buffer)
{
	m_graphicsDescriptorSets[rootIndex].SetUAV(offset, buffer);
	m_bAnyGraphicsDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetGraphicsUAV(int rootIndex, int offset, const Texture& texture)
{
	m_graphicsDescriptorSets[rootIndex].SetUAV(offset, texture);
	m_bAnyGraphicsDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetGraphicsCBV(int rootIndex, int offset, const ConstantBuffer& buffer)
{
	m_graphicsDescriptorSets[rootIndex].SetCBV(offset, buffer);
	m_bAnyGraphicsDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetComputeSRV(int rootIndex, int offset, const ColorBuffer& buffer)
{
	m_computeDescriptorSets[rootIndex].SetSRV(offset, buffer);
	m_bAnyComputeDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetComputeSRV(int rootIndex, int offset, const DepthBuffer& buffer, bool depthSrv)
{
	m_computeDescriptorSets[rootIndex].SetSRV(offset, buffer, depthSrv);
	m_bAnyComputeDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetComputeSRV(int rootIndex, int offset, const StructuredBuffer& buffer)
{
	m_computeDescriptorSets[rootIndex].SetSRV(offset, buffer);
	m_bAnyComputeDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetComputeSRV(int rootIndex, int offset, const Texture& texture)
{
	m_computeDescriptorSets[rootIndex].SetSRV(offset, texture);
	m_bAnyComputeDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetComputeUAV(int rootIndex, int offset, const ColorBuffer& buffer)
{
	m_computeDescriptorSets[rootIndex].SetUAV(offset, buffer);
	m_bAnyComputeDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetComputeUAV(int rootIndex, int offset, const DepthBuffer& buffer)
{
	m_computeDescriptorSets[rootIndex].SetUAV(offset, buffer);
	m_bAnyComputeDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetComputeUAV(int rootIndex, int offset, const StructuredBuffer& buffer)
{
	m_computeDescriptorSets[rootIndex].SetUAV(offset, buffer);
	m_bAnyComputeDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetComputeUAV(int rootIndex, int offset, const Texture& texture)
{
	m_computeDescriptorSets[rootIndex].SetUAV(offset, texture);
	m_bAnyComputeDescriptorsDirty = true;
}


void DynamicDescriptorPool::SetComputeCBV(int rootIndex, int offset, const ConstantBuffer& buffer)
{
	m_computeDescriptorSets[rootIndex].SetCBV(offset, buffer);
	m_bAnyComputeDescriptorsDirty = true;
}


void DynamicDescriptorPool::CommitGraphicsDescriptorSets(VkCommandBuffer commandList, VkPipelineLayout pipelineLayout)
{
	if (!m_bAnyGraphicsDescriptorsDirty)
		return;

	CommitDescriptorSetsInternal(commandList, pipelineLayout, m_graphicsDescriptorSets, VK_PIPELINE_BIND_POINT_GRAPHICS);

	m_bAnyGraphicsDescriptorsDirty = false;
}


void DynamicDescriptorPool::CommitComputeDescriptorSets(VkCommandBuffer commandList, VkPipelineLayout pipelineLayout)
{
	if (!m_bAnyComputeDescriptorsDirty)
		return;

	CommitDescriptorSetsInternal(commandList, pipelineLayout, m_computeDescriptorSets, VK_PIPELINE_BIND_POINT_COMPUTE);

	m_bAnyComputeDescriptorsDirty = false;
}


void DynamicDescriptorPool::RetireCurrentPool()
{
	// Don't retire unused pools
	if (m_curPoolAllocation.IsEmpty())
	{
		assert(m_curDescriptorPool == nullptr);
		return;
	}

	assert(m_curDescriptorPool != nullptr);
	m_retiredPools.push_back(m_curDescriptorPool);
	m_curDescriptorPool = nullptr;
	m_curPoolAllocation.Reset();
	m_curNumDescriptorSets = 0;
	m_bAnyGraphicsDescriptorsDirty = false;
	m_bAnyComputeDescriptorsDirty = false;
}


void DynamicDescriptorPool::RetireUsedPools(uint64_t fenceValue)
{
	DiscardDescriptorPools(fenceValue, m_retiredPools);
	m_retiredPools.clear();
}


void DynamicDescriptorPool::CommitDescriptorSetsInternal(VkCommandBuffer commandList, VkPipelineLayout pipelineLayout, array<DynamicDescriptorSet, 8>& descriptorSets, VkPipelineBindPoint bindPoint)
{
	if (m_curDescriptorPool == nullptr)
		m_curDescriptorPool = RequestDescriptorPool();

	// Determine how many new descriptor sets we'll need
	uint32_t numNewDescriptorSets = 0;
	for (auto& descriptorSet : descriptorSets)
	{
		if (descriptorSet.IsDirty())
			++numNewDescriptorSets;
	}

	if (numNewDescriptorSets == 0)
		return;

	bool bNeedNewPool = false;

	if (numNewDescriptorSets + m_curNumDescriptorSets >= s_maxDescriptorSets)
		bNeedNewPool = true;

	m_commitAllocation.Reset();

	if (!bNeedNewPool)
	{
		// Determine if our total requested descriptor allocation will fit in the current pool
		for (auto& descriptorSet : descriptorSets)
		{
			if (descriptorSet.IsDirty())
			{
				m_commitAllocation += descriptorSet.m_allocation;
			}
		}

		if (!(m_commitAllocation + m_curPoolAllocation < s_maxAllocationPerPool))
			bNeedNewPool = true;
	}

	if (bNeedNewPool)
	{
		RetireCurrentPool();
		m_curDescriptorPool = RequestDescriptorPool();
		assert(m_curDescriptorPool != nullptr);
	}

	// OK, now we have a pool that will fit everything

	// Allocate the new descriptor sets
	
	{
		int curSet = 0;
		for (int i = 0; i < 8; ++i)
		{
			m_commitDirtyList[i] = descriptorSets[i].IsDirty();
			if (descriptorSets[i].IsDirty())
			{
				m_commitLayouts[curSet] = descriptorSets[i].m_layout;
				++curSet;
			}
		}

		VkDescriptorSetAllocateInfo allocInfo{ VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO };
		allocInfo.descriptorPool = *m_curDescriptorPool;
		allocInfo.descriptorSetCount = numNewDescriptorSets;
		allocInfo.pSetLayouts = m_commitLayouts.data();

		ThrowIfFailed(vkAllocateDescriptorSets(GetDevice(), &allocInfo, m_commitDescriptorSets.data()));
	}

	// Now, write the staged descriptors
	{
		uint32_t curDescriptor = 0;
		uint32_t curDescriptorSet = 0;
		for (auto& descriptorSet : descriptorSets)
		{
			if (!descriptorSet.IsDirty())
				continue;

			unsigned long setBit{ 0 };
			while (_BitScanForward(&setBit, descriptorSet.m_dirtyBits))
			{
				m_commitWriteDescriptors[curDescriptor] = descriptorSet.m_writeDescriptorSets[setBit];
				m_commitWriteDescriptors[curDescriptor].dstSet = m_commitDescriptorSets[curDescriptorSet];
				descriptorSet.m_dirtyBits &= ~(1 << setBit);
				++curDescriptor;
			}

			++curDescriptorSet;
		}

		vkUpdateDescriptorSets(
			GetDevice(),
			curDescriptor,
			m_commitWriteDescriptors.data(),
			0,
			nullptr);
	}

	// Bind the descriptor sets to the pipeline
	{
		uint32_t curDescriptorSet = 0;
		for (int i = 0; i < 8; ++i)
		{
			if (!m_commitDirtyList[i])
				continue;

			vkCmdBindDescriptorSets(
				commandList,
				bindPoint,
				pipelineLayout,
				i,
				1,
				&m_commitDescriptorSets[curDescriptorSet++],
				0,
				nullptr);
		}
	}

	// Record the new numbers of descriptor sets and descriptors allocated from the current pool
	m_curNumDescriptorSets += numNewDescriptorSets;
	m_curPoolAllocation += m_commitAllocation;

	// Invalidate the descriptor sets
	for (auto& descriptorSet : descriptorSets)
	{
		if (descriptorSet.IsDirty())
		{
			descriptorSet.Invalidate();
		}
	}
}


UVkDescriptorPool* DynamicDescriptorPool::RequestDescriptorPool()
{
	lock_guard<mutex> lockGuard(s_mutex);

	while (!s_retiredDescriptorPools.empty() && g_commandManager.IsFenceComplete(s_retiredDescriptorPools.front().first))
	{
		s_availableDescriptorPools.push(s_retiredDescriptorPools.front().second);
		s_retiredDescriptorPools.pop();
	}

	if (!s_availableDescriptorPools.empty())
	{
		UVkDescriptorPool* pool = s_availableDescriptorPools.front();
		s_availableDescriptorPools.pop();
		vkResetDescriptorPool(GetDevice(), *pool, 0);
		return pool;
	}
	else
	{
		VkDescriptorPoolSize typeCounts[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, s_maxAllocationPerPool.numSamplers },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, s_maxAllocationPerPool.numCombinedImageSamplers },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, s_maxAllocationPerPool.numSampledImages },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, s_maxAllocationPerPool.numStorageImages },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, s_maxAllocationPerPool.numUniformTexelBuffers },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, s_maxAllocationPerPool.numStorageTexelBuffers },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, s_maxAllocationPerPool.numUniformBuffers },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, s_maxAllocationPerPool.numStorageBuffers },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, s_maxAllocationPerPool.numDynamicUniformBuffers },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, s_maxAllocationPerPool.numDynamicStorageBuffers },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, s_maxAllocationPerPool.numInputAttachments }
		};

		VkDescriptorPoolCreateInfo createInfo;
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.flags = 0;
		createInfo.maxSets = s_maxDescriptorSets;
		createInfo.poolSizeCount = _countof(typeCounts);
		createInfo.pPoolSizes = typeCounts;

		Microsoft::WRL::ComPtr<UVkDescriptorPool> descriptorPool;
		ThrowIfFailed(g_graphicsDevice->CreateDescriptorPool(createInfo, &descriptorPool));
		s_descriptorPools.emplace_back(descriptorPool);
		return descriptorPool.Get();
	}
}


void DynamicDescriptorPool::DiscardDescriptorPools(uint64_t fenceValueForReset, const vector<UVkDescriptorPool*>& usedPools)
{
	lock_guard<mutex> lockGuard(s_mutex);

	for (auto iter = usedPools.begin(); iter != usedPools.end(); ++iter)
	{
		s_retiredDescriptorPools.push(make_pair(fenceValueForReset, *iter));
	}
}