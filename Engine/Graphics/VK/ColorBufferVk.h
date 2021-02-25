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
#include "Graphics\PixelBuffer.h"


namespace Kodiak
{

class ColorBuffer : public PixelBuffer
{
public:
	ColorBuffer(Color clearColor = Color(0.0f, 0.0f, 0.0f, 0.0f));
	~ColorBuffer();

	// Create a color buffer from a swap chain buffer.  Unordered access is restricted.
	void CreateFromSwapChain(const std::string& name, UVkImage* uimage, uint32_t width, uint32_t height, Format format);

	// Create a color buffer.
	void Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numMips, Format format);

	void Create3D(const std::string& name, uint32_t width, uint32_t height, uint32_t depth, Format format);

	// Create a color buffer.
	void CreateArray(const std::string& name, uint32_t width, uint32_t height, uint32_t arrayCount, Format format);

	// Get pre-created CPU-visible descriptor handles
	VkImageView GetImageViewRTV() const { return m_imageViewRTV->Get(); }
	VkImageView GetImageViewSRV() const { return m_imageViewSRV->Get(); }
	const VkDescriptorImageInfo* GetSRVImageInfoPtr() const { return &m_imageInfoSRV; }
	const VkDescriptorImageInfo* GetUAVImageInfoPtr() const { return &m_imageInfoUAV; }

	void SetClearColor(Color clearColor) { m_clearColor = clearColor; }
	Color GetClearColor() const { return m_clearColor; }

	void SetMsaaMode(uint32_t numColorSamples, uint32_t numCoverageSamples)
	{
		assert(numCoverageSamples >= numColorSamples);
		m_fragmentCount = numColorSamples;
		m_numSamples = numCoverageSamples;
	}

protected:
	// Compute the number of texture levels needed to reduce to 1x1.  This uses
	// _BitScanReverse to find the highest set bit.  Each dimension reduces by
	// half and truncates bits.  The dimension 256 (0x100) has 9 mip levels, same
	// as the dimension 511 (0x1FF).
	static inline uint32_t ComputeNumMips(uint32_t width, uint32_t height)
	{
		uint32_t highBit{ 0 };
		_BitScanReverse((unsigned long*)&highBit, width | height);
		return highBit + 1;
	}

	void CreateDerivedViews(Format format, uint32_t arraySize, uint32_t numMips);

protected:
	Microsoft::WRL::ComPtr<UVkImageView> m_imageViewRTV{ nullptr };
	Microsoft::WRL::ComPtr<UVkImageView> m_imageViewSRV{ nullptr };
	VkDescriptorImageInfo m_imageInfoSRV{};
	VkDescriptorImageInfo m_imageInfoUAV{};
	Color m_clearColor;
	uint32_t m_numMipMaps{ 0 }; // number of texture sublevels
	uint32_t m_fragmentCount{ 1 };
};

using ColorBufferPtr = std::shared_ptr<ColorBuffer>;

} // namespace Kodiak