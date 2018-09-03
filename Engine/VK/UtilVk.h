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

} // namespace Kodiak