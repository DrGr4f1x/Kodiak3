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

#include "Graphics\VK\GpuImageVk.h"
#include "Graphics\VK\GpuResourceVk.h"

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

protected:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_arraySize{ 0 };
	uint32_t m_numMips{ 1 };
	uint32_t m_numSamples{ 1 };
	Format m_format{ Format::Unknown };
};

} // namespace Kodiak