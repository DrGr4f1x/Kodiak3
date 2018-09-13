//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "DynamicDescriptorPoolVk.h"

#include "CommandContextVk.h"
#include "CommandListManagerVk.h"
#include "GraphicsDevice.h"
#include "RootSignatureVk.h"

using namespace Kodiak;
using namespace std;

std::mutex DynamicDescriptorPool::sm_mutex;
std::vector<VkDescriptorPool> DynamicDescriptorPool::sm_descriptorPool;
std::queue<std::pair<std::shared_ptr<Fence>, VkDescriptorPool>> DynamicDescriptorPool::sm_retiredDescriptorPools;
std::queue<VkDescriptorPool> DynamicDescriptorPool::sm_availableDescriptorPools;


DynamicDescriptorPool::DynamicDescriptorPool(CommandContext& owningContext)
	: m_owningContext(owningContext)
{}


void DynamicDescriptorPool::DestroyAll()
{
	lock_guard<mutex> CS(sm_mutex);

	VkDevice device = GetDevice();

	for (auto& pool : sm_descriptorPool)
	{
		vkDestroyDescriptorPool(device, pool, nullptr);
	}
	sm_descriptorPool.clear();

	while (!sm_retiredDescriptorPools.empty())
	{
		sm_retiredDescriptorPools.pop();
	}
}


void DynamicDescriptorPool::CleanupUsedPools(shared_ptr<Fence> fence)
{
	RetireCurrentPool();
	RetireUsedPools(fence);
}


void DynamicDescriptorPool::RetireCurrentPool()
{
	// Don't retire unused pools.
	if (m_totalDescriptorsUsed == 0)
	{
		assert(m_currentPool == nullptr);
		return;
	}

	assert(m_currentPool != nullptr);
	m_retiredPools.push_back(m_currentPool);
	m_currentPool = nullptr;
	m_totalDescriptorsUsed = 0;
}


void DynamicDescriptorPool::RetireUsedPools(std::shared_ptr<Fence> fence)
{
	DiscardDescriptorPools(fence, m_retiredPools);
	m_retiredPools.clear();
}


VkDescriptorPool DynamicDescriptorPool::GetDescriptorPool()
{
	if (m_currentPool == VK_NULL_HANDLE)
	{
		assert(m_totalDescriptorsUsed == 0);
		m_currentPool = RequestDescriptorPool();
	}
	return m_currentPool;
}


void DynamicDescriptorPool::BindImmutableSamplers(VkPipelineBindPoint bindPoint, const RootSignature& rootSig)
{
	if (rootSig.m_staticSamplerSet != VK_NULL_HANDLE)
	{
		vkCmdBindDescriptorSets(
			m_owningContext.m_commandList,
			bindPoint,
			m_pipelineLayout,
			rootSig.m_staticSamplerSetIndex,
			1,
			&rootSig.m_staticSamplerSet,
			0,
			nullptr);
	}
}


VkDescriptorPool DynamicDescriptorPool::RequestDescriptorPool()
{
	lock_guard<mutex> CS(sm_mutex);

	while (!sm_retiredDescriptorPools.empty() && sm_retiredDescriptorPools.front().first->IsComplete())
	{
		sm_availableDescriptorPools.push(sm_retiredDescriptorPools.front().second);
		sm_retiredDescriptorPools.pop();
	}

	VkDevice device = GetDevice();

	if (!sm_availableDescriptorPools.empty())
	{
		VkDescriptorPool pool = sm_availableDescriptorPools.front();
		sm_availableDescriptorPools.pop();

		// Free all descriptors previously allocated from the pool
		ThrowIfFailed(vkResetDescriptorPool(device, pool, 0));

		return pool;
	}
	else
	{
		VkDescriptorPoolSize typeCounts[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, kMaxSamplers },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, kMaxCombinedImageSamplers },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, kMaxSampledImages },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, kMaxStorageImages },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, kMaxUniformTexelBuffers },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, kMaxStorageTexelBuffers },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, kMaxUniformBuffers },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, kMaxStorageBuffers },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, kMaxDynamicUniformBuffers },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, kMaxDynamicStorageBuffers },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, kMaxInputAttachments }
		};

		VkDescriptorPoolCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		createInfo.pNext = nullptr;
		createInfo.poolSizeCount = _countof(typeCounts);
		createInfo.pPoolSizes = typeCounts;
		createInfo.maxSets = kMaxDescriptorSets;

		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		ThrowIfFailed(vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool));

		sm_descriptorPool.emplace_back(descriptorPool);

		return descriptorPool;
	}
}


void DynamicDescriptorPool::DiscardDescriptorPools(std::shared_ptr<Fence> fence, const vector<VkDescriptorPool>& usedPools)
{
	lock_guard<mutex> CS(sm_mutex);
	for (auto iter = usedPools.begin(); iter != usedPools.end(); ++iter)
	{
		sm_retiredDescriptorPools.push(make_pair(fence, *iter));
	}
}


void DynamicDescriptorPool::CopyAndBindStagedDescriptors(DescriptorHandleCache& handleCache, VkCommandBuffer cmdBuffer, bool isCompute)
{
	vector<VkDescriptorSet> descriptorSets;
	descriptorSets.reserve(8);

	VkDevice device = GetDevice();

	uint32_t rootIndex = 0;
	uint32_t staleParams = handleCache.m_staleDescriptorSetBitMap;
	while (BitScanForward((unsigned long*)&rootIndex, staleParams))
	{
		staleParams ^= (1 << rootIndex);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.pNext = nullptr;
		allocInfo.descriptorPool = GetDescriptorPool();
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &handleCache.m_descriptorLayouts[rootIndex];

		// Attempt to allocate a descriptor set
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
		//VkResult res = vkAllocateDescriptorSets(g_Device, &allocInfo, &descriptorSet);

		++m_totalDescriptorsUsed;

		if (m_totalDescriptorsUsed >= kMaxDescriptorSets)
		{
			// Try again with a new descriptor pool
			RetireCurrentPool();
			allocInfo.descriptorPool = GetDescriptorPool();
		}

		ThrowIfFailed(vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet));


		// Setup the descriptor set
		auto& descriptorSetCache = handleCache.m_descriptorSetCache[rootIndex];
		for (uint32_t i = 0; i < descriptorSetCache.rangeCount; ++i)
		{
			VkWriteDescriptorSet writeDescriptorSet = {};
			writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescriptorSet.dstSet = descriptorSet;

			auto& rangeDesc = descriptorSetCache.ranges[i];

			// Iterate over the assigned descriptor bits in this descriptor range
			uint32_t rangeIndex = 0;
			while (BitScanForward((unsigned long*)&rangeIndex, rangeDesc.assignedHandlesBitmap))
			{
				rangeDesc.assignedHandlesBitmap ^= (1 << rangeIndex);

				writeDescriptorSet.descriptorCount = 1;
				writeDescriptorSet.dstBinding = rangeDesc.offset + rangeIndex;

				switch (rangeDesc.type)
				{
				case DescriptorType::CBV:
					writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					writeDescriptorSet.pBufferInfo = rangeDesc.bufferHandleStart + rangeDesc.offset + rangeIndex;
					break;

				case DescriptorType::TextureSRV:
					writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
					writeDescriptorSet.pImageInfo = rangeDesc.imageHandleStart + rangeDesc.offset + rangeIndex;
					break;

				case DescriptorType::TextureUAV:
					writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					writeDescriptorSet.pImageInfo = rangeDesc.imageHandleStart + rangeDesc.offset + rangeIndex;
					break;

				case DescriptorType::StructuredBufferSRV:
				case DescriptorType::StructuredBufferUAV:
					writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					writeDescriptorSet.pBufferInfo = rangeDesc.bufferHandleStart + rangeDesc.offset + rangeIndex;
					break;

				case DescriptorType::TypedBufferSRV:
					writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
					writeDescriptorSet.pTexelBufferView = rangeDesc.texelBufferHandleStart + rangeDesc.offset + rangeIndex;
					break;

				case DescriptorType::TypedBufferUAV:
					writeDescriptorSet.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
					writeDescriptorSet.pTexelBufferView = rangeDesc.texelBufferHandleStart + rangeDesc.offset + rangeIndex;
					break;
				}

				vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);
			}
		}

		descriptorSets.push_back(descriptorSet);
	}
	handleCache.m_staleDescriptorSetBitMap = 0;

	vkCmdBindDescriptorSets(
		cmdBuffer,
		isCompute ? VK_PIPELINE_BIND_POINT_COMPUTE : VK_PIPELINE_BIND_POINT_GRAPHICS,
		m_pipelineLayout,
		0,
		static_cast<uint32_t>(descriptorSets.size()),
		descriptorSets.data(),
		0,
		nullptr);
}


void DynamicDescriptorPool::DescriptorHandleCache::Initialize()
{
	for (auto& desc : m_imageDescriptors)
	{
		desc.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		desc.imageView = VK_NULL_HANDLE;
		desc.sampler = VK_NULL_HANDLE;
	}

	for (auto& desc : m_bufferDescriptors)
	{
		desc.buffer = VK_NULL_HANDLE;
		desc.offset = 0;
		desc.range = VK_WHOLE_SIZE;
	}

	for (auto& desc : m_texelBufferDescriptors)
	{
		desc = VK_NULL_HANDLE;
	}
}


void DynamicDescriptorPool::DescriptorHandleCache::ClearCache()
{
	m_assignedDescriptorSetBitMap = 0;
	m_staleDescriptorSetBitMap = 0;
	for (uint32_t i = 0; i < kMaxNumDescriptorSets; ++i)
	{
		m_descriptorLayouts[i] = VK_NULL_HANDLE;
	}
	maxCachedImageDescriptors = 0;
	maxCachedBufferDescriptors = 0;
	maxCachedTexelBufferDescriptors = 0;
}


namespace
{

inline bool IsTextureDescriptor(const DescriptorType type)
{
	return (type == DescriptorType::TextureSRV || type == DescriptorType::TextureUAV);
}


inline bool IsBufferDescriptor(const DescriptorType type)
{
	return (type == DescriptorType::CBV || type == DescriptorType::StructuredBufferSRV || type == DescriptorType::StructuredBufferUAV);
}


inline bool IsTypedBufferDescriptor(const DescriptorType type)
{
	return (type == DescriptorType::TypedBufferSRV || type == DescriptorType::TypedBufferUAV);
}

} // anonymous namespace


void DynamicDescriptorPool::DescriptorHandleCache::StageDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkDescriptorImageInfo handles[])
{
	DescriptorSetCache& descriptorSet = m_descriptorSetCache[rootIndex];
	for (uint32_t rangeIndex = 0; rangeIndex < descriptorSet.rangeCount; ++rangeIndex)
	{
		auto& rangeDesc = descriptorSet.ranges[rangeIndex];

		if (!IsTextureDescriptor(rangeDesc.type)) continue;

		if (offset >= rangeDesc.offset && offset < (rangeDesc.offset + rangeDesc.rangeSize))
		{
			assert(numHandles <= rangeDesc.rangeSize);

			VkDescriptorImageInfo* copyDest = rangeDesc.imageHandleStart + offset;
			for (uint32_t i = 0; i < numHandles; ++i)
			{
				copyDest[i] = handles[i];
				rangeDesc.assignedHandlesBitmap |= (1 << i) << offset;
			}
			descriptorSet.assignedImageHandlesBitMap |= ((1 << numHandles) - 1) << offset;
			m_staleDescriptorSetBitMap |= (1 << rootIndex);
		}
	}
}


void DynamicDescriptorPool::DescriptorHandleCache::StageDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkDescriptorBufferInfo handles[])
{
	DescriptorSetCache& descriptorSet = m_descriptorSetCache[rootIndex];
	for (uint32_t rangeIndex = 0; rangeIndex < descriptorSet.rangeCount; ++rangeIndex)
	{
		auto& rangeDesc = descriptorSet.ranges[rangeIndex];

		if (!IsBufferDescriptor(rangeDesc.type)) continue;

		if (offset >= rangeDesc.offset && offset < (rangeDesc.offset + rangeDesc.rangeSize))
		{
			assert(numHandles <= rangeDesc.rangeSize);

			VkDescriptorBufferInfo* copyDest = rangeDesc.bufferHandleStart + offset;
			for (uint32_t i = 0; i < numHandles; ++i)
			{
				copyDest[i] = handles[i];
				rangeDesc.assignedHandlesBitmap |= (1 << i);
			}
			descriptorSet.assignedBufferHandlesBitMap |= ((1 << numHandles) - 1) << offset;
			m_staleDescriptorSetBitMap |= (1 << rootIndex);
		}
	}
}


void DynamicDescriptorPool::DescriptorHandleCache::StageDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkBufferView handles[])
{
	DescriptorSetCache& descriptorSet = m_descriptorSetCache[rootIndex];
	for (uint32_t rangeIndex = 0; rangeIndex < descriptorSet.rangeCount; ++rangeIndex)
	{
		auto& rangeDesc = descriptorSet.ranges[rangeIndex];

		if (!IsTypedBufferDescriptor(rangeDesc.type)) continue;

		if (offset >= rangeDesc.offset && offset < (rangeDesc.offset + rangeDesc.rangeSize))
		{
			assert(numHandles <= rangeDesc.rangeSize);

			VkBufferView* copyDest = rangeDesc.texelBufferHandleStart + offset;
			for (uint32_t i = 0; i < numHandles; ++i)
			{
				copyDest[i] = handles[i];
				rangeDesc.assignedHandlesBitmap |= (1 << i);
			}
			descriptorSet.assignedTexelBufferHandlesBitMap |= ((1 << numHandles) - 1) << offset;
			m_staleDescriptorSetBitMap |= (1 << rootIndex);
		}
	}
}


void DynamicDescriptorPool::DescriptorHandleCache::ParseRootSignature(const RootSignature& rootSig)
{
	ClearCache();

	uint32_t currentImageOffset = 0;
	uint32_t currentBufferOffset = 0;
	uint32_t currentTexelBufferOffset = 0;

	auto numParameters = rootSig.m_numParameters;
	assert(numParameters <= kMaxDescriptorSets);

	for (uint32_t rootIndex = 0; rootIndex < numParameters; ++rootIndex)
	{
		const RootParameter& param = rootSig[rootIndex];

		auto type = param.GetType();

		// Mark this descriptor set as assigned
		m_assignedDescriptorSetBitMap |= (1 << rootIndex);
		m_descriptorLayouts[rootIndex] = param.GetLayout();

		// Fill out info about this descriptor set
		DescriptorSetCache& descriptorSet = m_descriptorSetCache[rootIndex];

		descriptorSet.assignedBufferHandlesBitMap = 0;
		descriptorSet.assignedImageHandlesBitMap = 0;
		descriptorSet.assignedTexelBufferHandlesBitMap = 0;

		switch (type)
		{
		case RootParameterType::RootCBV:
			descriptorSet.rangeCount = 1;
			descriptorSet.ranges[0].type = DescriptorType::CBV;
			descriptorSet.ranges[0].rangeSize = 1;
			descriptorSet.ranges[0].offset = 0;
			descriptorSet.ranges[0].bufferHandleStart = &m_bufferDescriptors[0] + currentBufferOffset;
			currentBufferOffset++;
			break;

		case RootParameterType::DescriptorTable:
			const uint32_t numRanges = param.GetNumRanges();
			descriptorSet.rangeCount = numRanges;
			uint32_t currentOffset = 0;
			for (uint32_t rangeIndex = 0; rangeIndex < numRanges; ++rangeIndex)
			{
				const auto& rangeDesc = param.GetRangeDesc(rangeIndex);
				descriptorSet.ranges[rangeIndex].type = rangeDesc.type;
				descriptorSet.ranges[rangeIndex].rangeSize = rangeDesc.numDescriptors;
				descriptorSet.ranges[rangeIndex].offset = currentOffset;
				currentOffset += rangeDesc.numDescriptors;
				
				switch (rangeDesc.type)
				{
				case DescriptorType::CBV:
				case DescriptorType::StructuredBufferSRV:
				case DescriptorType::StructuredBufferUAV:
					descriptorSet.ranges[rangeIndex].bufferHandleStart = &m_bufferDescriptors[0] + currentBufferOffset;
					currentBufferOffset += rangeDesc.numDescriptors;
					break;

				case DescriptorType::TextureSRV:
				case DescriptorType::TextureUAV:
					descriptorSet.ranges[rangeIndex].imageHandleStart = &m_imageDescriptors[0] + currentImageOffset;
					currentImageOffset += rangeDesc.numDescriptors;
					break;

				case DescriptorType::TypedBufferSRV:
				case DescriptorType::TypedBufferUAV:
					descriptorSet.ranges[rangeIndex].texelBufferHandleStart = &m_texelBufferDescriptors[0] + currentTexelBufferOffset;
					currentTexelBufferOffset += rangeDesc.numDescriptors;
					break;
				}
			}
			break;
		}
	}

	maxCachedImageDescriptors = currentImageOffset;
	assert_msg(maxCachedImageDescriptors <= kMaxDescriptors, "Exceeded user-supplied maximum image cache size");

	maxCachedBufferDescriptors = currentBufferOffset;
	assert_msg(maxCachedBufferDescriptors <= kMaxDescriptors, "Exceeded user-supplied maximum buffer cache size");

	maxCachedTexelBufferDescriptors = currentTexelBufferOffset;
	assert_msg(maxCachedTexelBufferDescriptors <= kMaxDescriptors, "Exceeded user-supplied maximum texel buffer cache size");
}