// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

#include "GpuResourceVk.h"

namespace Kodiak
{

class PixelBuffer : public GpuResource
{
public:
	void Destroy();

	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	uint32_t GetDepth() const { return m_arraySize; }
	VkFormat GetFormat() const { return m_format; }

protected:
	VkImageCreateInfo DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, VkFormat format, VkImageUsageFlags flags);

	void CreateTextureResource(const std::string& name, const VkImageCreateInfo& imageCreateInfo);

protected:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_arraySize{ 0 };
	VkFormat m_format{ VK_FORMAT_UNDEFINED };

	VkImage			m_image{ VK_NULL_HANDLE };
	VkImageLayout	m_layout{ VK_IMAGE_LAYOUT_GENERAL };
	VkAccessFlags	m_accessFlags{ 0 };
};

} // namespace Kodiak