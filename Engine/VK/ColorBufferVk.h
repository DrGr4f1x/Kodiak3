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

#include "Color.h"
#include "PixelBufferVk.h"

namespace Kodiak
{

class ColorBuffer : public PixelBuffer
{
public:
	ColorBuffer(Color clearColor = Color(0.0f, 0.0f, 0.0f, 0.0f))
		: m_clearColor(clearColor), m_numMipMaps(0)
	{}

	~ColorBuffer() { Destroy(); }

	void Destroy();

	// Create a color buffer from a swap chain buffer.  Unordered access is restricted.
	void CreateFromSwapChain(const std::string& name, VkImage baseImage, uint32_t width, uint32_t height, Format format);

	void Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numMips, Format format);

	// Get pre-created CPU-visible descriptor handles
	VkImageView GetSRV() const { return m_imageView; }
	VkImageView GetRTV() const { return m_imageView; }

	const Color& GetClearColor() const { return m_clearColor; }

protected:
	// Compute the number of texture levels needed to reduce to 1x1.  This uses
	// _BitScanReverse to find the highest set bit.  Each dimension reduces by
	// half and truncates bits.  The dimension 256 (0x100) has 9 mip levels, same
	// as the dimension 511 (0x1FF).
	static inline uint32_t ComputeNumMips(uint32_t width, uint32_t height)
	{
		uint32_t highBit;
		_BitScanReverse((unsigned long*)&highBit, width | height);
		return highBit + 1;
	}

protected:
	Color			m_clearColor;
	VkImageView		m_imageView{ VK_NULL_HANDLE };
	uint32_t		m_numMipMaps;
	bool			m_ownsImage{ true };

private:
	void CreateDerivedViews(Format format, uint32_t arraySize, uint32_t numMips = 1);
};

using ColorBufferPtr = std::shared_ptr<ColorBuffer>;
using ColorBufferUPtr = std::unique_ptr<ColorBuffer>;

} // namespace Kodiak