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

#include "GpuResource.h"

namespace Kodiak
{

class PixelBuffer : public GpuResource
{
public:
	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	uint32_t GetDepth() const { return m_arraySize; }
	uint32_t GetArraySize() const { return m_arraySize; }
	uint32_t GetNumMips() const { return m_numMips; }
	uint32_t GetNumSamples() const { return m_numSamples; }
	Format GetFormat() const { return m_format; }

#if VK
	void SetLayout(VkImageLayout layout) { m_layout = layout; }
	VkImageLayout GetLayout() const { return m_layout; }

	void SetAccessFlags(VkAccessFlags flags) { m_accessFlags = flags; }
	VkAccessFlags GetAccessFlags() const { return m_accessFlags; }
#endif

protected:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_arraySize{ 0 };
	uint32_t m_numMips{ 1 };
	uint32_t m_numSamples{ 1 };
	Format m_format{ Format::Unknown };

	// TODO - Wrap this up?
#if VK
	VkImageLayout	m_layout{ VK_IMAGE_LAYOUT_GENERAL };
	VkAccessFlags	m_accessFlags{ 0 };
#endif

};

} // namespace Kodiak