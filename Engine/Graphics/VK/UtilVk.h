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

VkImageCreateInfo DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips,
	uint32_t numSamples, Format format, VkImageUsageFlags usageFlags);

ResourceHandle CreateTextureResource(const std::string& name, const VkImageCreateInfo& imageCreateInfo);

VkSampleCountFlagBits SamplesToFlags(uint32_t numSamples);
VkImageType GetImageType(ResourceType type);
VkImageViewType GetImageViewType(ResourceType type);
VkImageCreateFlagBits GetImageCreateFlags(ResourceType type);

VkImageLayout GetImageLayout(ResourceState state);
VkAccessFlagBits GetAccessMask(ResourceState state);
VkPipelineStageFlags GetShaderStageMask(ResourceState state, bool isSrc);
VkImageAspectFlags GetAspectFlagsFromFormat(Format format, bool ignoreStencil = false);

inline VkQueryType QueryHeapTypeToVulkan(QueryHeapType type)
{
	switch (type)
	{
	case QueryHeapType::Occlusion:
		return VK_QUERY_TYPE_OCCLUSION;

	case QueryHeapType::PipelineStats:
		return VK_QUERY_TYPE_PIPELINE_STATISTICS;

	case QueryHeapType::Timestamp:
		return VK_QUERY_TYPE_TIMESTAMP;

	default:
		assert(false);
		return VK_QUERY_TYPE_OCCLUSION;
	}
}


inline VkQueryType QueryTypeToVulkan(QueryType type)
{
	switch (type)
	{
	case QueryType::Occlusion:
		return VK_QUERY_TYPE_OCCLUSION;

	case QueryType::PipelineStats:
		return VK_QUERY_TYPE_PIPELINE_STATISTICS;

	case QueryType::Timestamp:
		return VK_QUERY_TYPE_TIMESTAMP;

	default:
		assert(false);
		return VK_QUERY_TYPE_OCCLUSION;
	}
}

} // namespace Kodiak