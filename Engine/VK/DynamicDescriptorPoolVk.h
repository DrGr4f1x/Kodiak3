//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "RootSignatureVk.h"

#if 0

namespace Kodiak
{

// Forward declarations
class CommandContext;
class Fence;


class DynamicDescriptorPool
{
public:
	DynamicDescriptorPool(CommandContext& owningContext);

	static void DestroyAll();

	void CleanupUsedPools(std::shared_ptr<Fence> fence);

	// Copy multiple handles into the cache area reserved for the specified root parameter.
	void SetGraphicsDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkDescriptorImageInfo handles[])
	{
		m_graphicsHandleCache.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
	}
	void SetGraphicsDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkDescriptorBufferInfo handles[])
	{
		m_graphicsHandleCache.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
	}
	void SetGraphicsDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkBufferView handles[])
	{
		m_graphicsHandleCache.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
	}

	void SetComputeDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkDescriptorImageInfo handles[])
	{
		m_computeHandleCache.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
	}
	void SetComputeDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkDescriptorBufferInfo handles[])
	{
		m_computeHandleCache.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
	}
	void SetComputeDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkBufferView handles[])
	{
		m_computeHandleCache.StageDescriptorHandles(rootIndex, offset, numHandles, handles);
	}

	// Deduce cache layout needed to support the descriptor sets needed by the root signature.
	void ParseGraphicsRootSignature(const RootSignature& rootSig)
	{
		m_pipelineLayout = rootSig.GetLayout();
		m_graphicsHandleCache.ParseRootSignature(rootSig);
	}

	void ParseComputeRootSignature(const RootSignature& rootSig)
	{
		m_pipelineLayout = rootSig.GetLayout();
		m_computeHandleCache.ParseRootSignature(rootSig);
	}

	// Upload any new descriptors in the cache to the shader-visible heap.
	inline void CommitGraphicsRootDescriptorSets(VkCommandBuffer commandList)
	{
		if (m_graphicsHandleCache.m_staleDescriptorSetBitMap != 0)
		{
			CopyAndBindStagedDescriptors(m_graphicsHandleCache, commandList, false);
		}
	}

	inline void CommitComputeRootDescriptorSets(VkCommandBuffer commandList)
	{
		if (m_graphicsHandleCache.m_staleDescriptorSetBitMap != 0)
		{
			CopyAndBindStagedDescriptors(m_graphicsHandleCache, commandList, true);
		}
	}

private:
	void RetireCurrentPool();
	void RetireUsedPools(std::shared_ptr<Fence> fence);

	VkDescriptorPool GetDescriptorPool();

	// Static methods
	static VkDescriptorPool RequestDescriptorPool();
	static void DiscardDescriptorPools(std::shared_ptr<Fence>, const std::vector<VkDescriptorPool>& usedPools);

private:
	CommandContext& m_owningContext;

	VkDescriptorPool m_currentPool{ VK_NULL_HANDLE };
	uint32_t m_totalDescriptorsUsed{ 0 };
	std::vector<VkDescriptorPool> m_retiredPools;
	VkDescriptorSet m_currentDescriptorSet;
	VkPipelineLayout m_pipelineLayout{ VK_NULL_HANDLE };

	// Static members
	static std::mutex sm_mutex;
	static std::vector<VkDescriptorPool> sm_descriptorPool;
	static std::queue<std::pair<std::shared_ptr<Fence>, VkDescriptorPool>> sm_retiredDescriptorPools;
	static std::queue<VkDescriptorPool> sm_availableDescriptorPools;
	static const uint32_t kMaxSamplers = 256;
	static const uint32_t kMaxCombinedImageSamplers = 1024;
	static const uint32_t kMaxSampledImages = 1024;
	static const uint32_t kMaxStorageImages = 256;
	static const uint32_t kMaxUniformTexelBuffers = 64;
	static const uint32_t kMaxStorageTexelBuffers = 64;
	static const uint32_t kMaxUniformBuffers = 1024;
	static const uint32_t kMaxStorageBuffers = 256;
	static const uint32_t kMaxDynamicUniformBuffers = 256;
	static const uint32_t kMaxDynamicStorageBuffers = 256;
	static const uint32_t kMaxInputAttachments = 256;
	static const uint32_t kMaxDescriptorSets = 1024;

	struct DescriptorTableRange
	{
		DescriptorType type;
		uint32_t offset{ 0 };
		uint32_t rangeSize{ 0 };
		uint32_t assignedHandlesBitmap{ 0 };
		union
		{
			VkDescriptorImageInfo* imageHandleStart;
			VkDescriptorBufferInfo* bufferHandleStart;
			VkBufferView* texelBufferHandleStart;
		};
	};
	// Describes a descriptor set entry:  a region of the handle cache and which handles have been set
	struct DescriptorSetCache
	{
		DescriptorSetCache() {}
		uint32_t rangeCount;
		const static uint32_t kMaxDescriptorRanges = 16;
		std::array<DescriptorTableRange, kMaxDescriptorRanges> ranges;

		uint32_t assignedImageHandlesBitMap{ 0 };
		uint32_t assignedBufferHandlesBitMap{ 0 };
		uint32_t assignedTexelBufferHandlesBitMap{ 0 };
	};

	struct DescriptorHandleCache
	{
		DescriptorHandleCache()
		{
			Initialize();
			ClearCache();
		}

		void StageDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkDescriptorImageInfo handles[]);
		void StageDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkDescriptorBufferInfo handles[]);
		void StageDescriptorHandles(uint32_t rootIndex, uint32_t offset, uint32_t numHandles, const VkBufferView handles[]);

		static const uint32_t kMaxNumDescriptorSets = 16;
		static const uint32_t kMaxDescriptors = 256;
		uint32_t m_assignedDescriptorSetBitMap{ 0 };
		uint32_t m_staleDescriptorSetBitMap{ 0 };

		std::array<DescriptorSetCache, kMaxDescriptorSets> m_descriptorSetCache;

		std::array<VkDescriptorSetLayout, kMaxNumDescriptorSets> m_descriptorLayouts;
		std::array<VkDescriptorImageInfo, kMaxDescriptors> m_imageDescriptors;
		std::array<VkDescriptorBufferInfo, kMaxDescriptors> m_bufferDescriptors;
		std::array<VkBufferView, kMaxDescriptors> m_texelBufferDescriptors;

		uint32_t maxCachedImageDescriptors{ 0 };
		uint32_t maxCachedBufferDescriptors{ 0 };
		uint32_t maxCachedTexelBufferDescriptors{ 0 };

		void Initialize();
		void ClearCache();
		void ParseRootSignature(const RootSignature& rootSig);
	};

	DescriptorHandleCache m_graphicsHandleCache;
	DescriptorHandleCache m_computeHandleCache;

	void CopyAndBindStagedDescriptors(DescriptorHandleCache& handleCache, VkCommandBuffer cmdList, bool isCompute);
};

} // namespace Kodiak

#endif