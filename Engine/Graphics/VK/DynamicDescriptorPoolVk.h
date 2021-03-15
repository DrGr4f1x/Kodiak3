//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#pragma once

namespace Kodiak
{

// Forward declarations
class ColorBuffer;
class CommandContext;
class ConstantBuffer;
class DepthBuffer;
class RootSignature;
class StructuredBuffer;
class Texture;


class DynamicDescriptorPool;


struct DescriptorAllocation
{
	uint32_t numSamplers{ 0 };
	uint32_t numCombinedImageSamplers{ 0 };
	uint32_t numSampledImages{ 0 };
	uint32_t numStorageImages{ 0 };
	uint32_t numUniformTexelBuffers{ 0 };
	uint32_t numStorageTexelBuffers{ 0 };
	uint32_t numUniformBuffers{ 0 };
	uint32_t numStorageBuffers{ 0 };
	uint32_t numDynamicUniformBuffers{ 0 };
	uint32_t numDynamicStorageBuffers{ 0 };
	uint32_t numInputAttachments{ 0 };
	
	void Reset()
	{
		numSamplers = 0;
		numCombinedImageSamplers = 0;
		numSampledImages = 0;
		numStorageImages = 0;
		numUniformTexelBuffers = 0;
		numStorageTexelBuffers = 0;
		numUniformBuffers = 0;
		numStorageBuffers = 0;
		numDynamicUniformBuffers = 0;
		numDynamicStorageBuffers = 0;
		numInputAttachments = 0;
	}

	bool IsEmpty() const
	{
		return
			(numSamplers == 0) &&
			(numCombinedImageSamplers == 0) &&
			(numSampledImages == 0) &&
			(numStorageImages == 0) &&
			(numUniformTexelBuffers == 0) &&
			(numStorageTexelBuffers == 0) &&
			(numUniformBuffers == 0) &&
			(numStorageBuffers == 0) &&
			(numDynamicUniformBuffers == 0) &&
			(numDynamicStorageBuffers == 0) &&
			(numInputAttachments == 0);
	}

	inline DescriptorAllocation operator+(const DescriptorAllocation& rhs) const
	{
		return DescriptorAllocation{
			numSamplers + rhs.numSamplers,
			numCombinedImageSamplers + rhs.numCombinedImageSamplers,
			numSampledImages + rhs.numSampledImages,
			numStorageImages + rhs.numStorageImages,
			numUniformTexelBuffers + rhs.numUniformTexelBuffers,
			numStorageTexelBuffers + rhs.numStorageTexelBuffers,
			numUniformBuffers + rhs.numUniformBuffers,
			numStorageBuffers + rhs.numStorageBuffers,
			numDynamicUniformBuffers + rhs.numDynamicUniformBuffers,
			numDynamicStorageBuffers + rhs.numDynamicStorageBuffers,
			numInputAttachments + rhs.numInputAttachments
		};
	}

	inline DescriptorAllocation& operator+=(const DescriptorAllocation& rhs)
	{
		*this = *this + rhs;
		return *this;
	}

	inline bool operator<(const DescriptorAllocation& rhs) const
	{
		return
			(numSamplers < rhs.numSamplers) &&
			(numCombinedImageSamplers < rhs.numCombinedImageSamplers) &&
			(numSampledImages < rhs.numSampledImages) &&
			(numStorageImages < rhs.numStorageImages) &&
			(numUniformTexelBuffers < rhs.numUniformTexelBuffers) &&
			(numStorageTexelBuffers < rhs.numStorageTexelBuffers) &&
			(numUniformBuffers < rhs.numUniformBuffers) &&
			(numStorageBuffers < rhs.numStorageBuffers) &&
			(numDynamicUniformBuffers < rhs.numDynamicUniformBuffers) &&
			(numDynamicStorageBuffers < rhs.numDynamicStorageBuffers) &&
			(numInputAttachments < rhs.numInputAttachments);
	}
};


class DynamicDescriptorSet
{
	friend class ComputeContext;
	friend class DynamicDescriptorPool;
	friend class GraphicsContext;

	enum { MaxDescriptors = 32 };

public:
	DynamicDescriptorSet();

	void Init(const RootSignature& rootSig, int rootParam);
	void Invalidate();

	bool IsInitialized() const { return m_bIsInitialized; }
	bool IsDirty() const { return m_dirtyBits != 0; }

	void SetSRV(int paramIndex, const ColorBuffer& buffer);
	void SetSRV(int paramIndex, const DepthBuffer& buffer, bool depthSrv = true);
	void SetSRV(int paramIndex, const StructuredBuffer& buffer);
	void SetSRV(int paramIndex, const Texture& texture);

	void SetUAV(int paramIndex, const ColorBuffer& buffer);
	void SetUAV(int paramIndex, const DepthBuffer& buffer);
	void SetUAV(int paramIndex, const StructuredBuffer& buffer);
	void SetUAV(int paramIndex, const Texture& texture);

	void SetCBV(int paramIndex, const ConstantBuffer& buffer);

private:
	void Update();

private:
	VkDescriptorSet m_descriptorSet{ VK_NULL_HANDLE };
	std::array<VkWriteDescriptorSet, MaxDescriptors> m_writeDescriptorSets;
	VkDescriptorSetLayout m_layout{ VK_NULL_HANDLE };
	DescriptorAllocation m_allocation{};
	uint32_t m_dirtyBits{ 0 };

	bool m_bIsInitialized{ false };
};


class DynamicDescriptorPool
{
public:
	DynamicDescriptorPool(CommandContext& owningContext);

	void ParseGraphicsRootSignature(const RootSignature& rootSig);
	void ParseComputeRootSignature(const RootSignature& rootSig);

	void CleanupUsedPools(uint64_t fenceValue);

	void SetGraphicsSRV(int rootIndex, int offset, const ColorBuffer& buffer);
	void SetGraphicsSRV(int rootIndex, int offset, const DepthBuffer& buffer, bool depthSrv = true);
	void SetGraphicsSRV(int rootIndex, int offset, const StructuredBuffer& buffer);
	void SetGraphicsSRV(int rootIndex, int offset, const Texture& texture);

	void SetGraphicsUAV(int rootIndex, int offset, const ColorBuffer& buffer);
	void SetGraphicsUAV(int rootIndex, int offset, const DepthBuffer& buffer);
	void SetGraphicsUAV(int rootIndex, int offset, const StructuredBuffer& buffer);
	void SetGraphicsUAV(int rootIndex, int offset, const Texture& texture);

	void SetGraphicsCBV(int rootIndex, int offset, const ConstantBuffer& buffer);

	void SetComputeSRV(int rootIndex, int offset, const ColorBuffer& buffer);
	void SetComputeSRV(int rootIndex, int offset, const DepthBuffer& buffer, bool depthSrv = true);
	void SetComputeSRV(int rootIndex, int offset, const StructuredBuffer& buffer);
	void SetComputeSRV(int rootIndex, int offset, const Texture& texture);

	void SetComputeUAV(int rootIndex, int offset, const ColorBuffer& buffer);
	void SetComputeUAV(int rootIndex, int offset, const DepthBuffer& buffer);
	void SetComputeUAV(int rootIndex, int offset, const StructuredBuffer& buffer);
	void SetComputeUAV(int rootIndex, int offset, const Texture& texture);

	void SetComputeCBV(int rootIndex, int offset, const ConstantBuffer& buffer);

	void CommitGraphicsDescriptorSets(VkCommandBuffer commandList, VkPipelineLayout pipelineLayout);
	void CommitComputeDescriptorSets(VkCommandBuffer commandList, VkPipelineLayout pipelineLayout);

private:
	void RetireCurrentPool();
	void RetireUsedPools(uint64_t fenceValue);

	void CommitDescriptorSetsInternal(VkCommandBuffer commandList, VkPipelineLayout pipelineLayout, std::array<DynamicDescriptorSet, 8>& descriptorSets, VkPipelineBindPoint bindPoint);

	static UVkDescriptorPool* RequestDescriptorPool();
	static void DiscardDescriptorPools(uint64_t fenceValueForReset, const std::vector<UVkDescriptorPool*>& usedPools);

private:
	static const DescriptorAllocation s_maxAllocationPerPool;
	static const uint32_t s_maxDescriptorSets;

	// Static members
	static std::mutex s_mutex;
	static std::vector<Microsoft::WRL::ComPtr<UVkDescriptorPool>> s_descriptorPools;
	static std::queue<std::pair<uint64_t, UVkDescriptorPool*>> s_retiredDescriptorPools;
	static std::queue<UVkDescriptorPool*> s_availableDescriptorPools;

	// Non-static members
	std::array<DynamicDescriptorSet, 8> m_graphicsDescriptorSets;
	std::array<DynamicDescriptorSet, 8> m_computeDescriptorSets;
	CommandContext& m_owningContext;
	DescriptorAllocation m_curPoolAllocation{};
	UVkDescriptorPool* m_curDescriptorPool{ nullptr };
	std::vector<UVkDescriptorPool*> m_retiredPools;
	uint32_t m_curNumDescriptorSets{ 0 };
	bool m_bAnyGraphicsDescriptorsDirty{ false };
	bool m_bAnyComputeDescriptorsDirty{ false };
};

} // namespace Kodiak