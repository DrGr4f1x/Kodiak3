//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Developed by Minigraph
//
// Author:  James Stanard 
//

#pragma once

#include "Color.h"

#include "PixelBuffer12.h"

namespace Kodiak
{

// Forward declarations
class CommandContext;


class ColorBuffer : public PixelBuffer
{
public:
	ColorBuffer(Color clearColor = Color(0.0f, 0.0f, 0.0f, 0.0f))
		: m_clearColor(clearColor)
	{
		m_srvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		m_srvHandle.ptr = D3D12_GPU_VIRTUAL_ADDRESS_UNKNOWN;
		std::memset(m_uavHandle, 0xFF, sizeof(m_uavHandle));
	}

	// Create a color buffer from a swap chain buffer.  Unordered access is restricted.
	void CreateFromSwapChain(const std::string& name, ID3D12Resource* baseResource);

	// Create a color buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void Create(const std::string& name, uint32_t width, uint32_t height, uint32_t numMips,	DXGI_FORMAT format);

	// Create a color buffer.  If an address is supplied, memory will not be allocated.
	// The vmem address allows you to alias buffers (which can be especially useful for
	// reusing ESRAM across a frame.)
	void CreateArray(const std::string& name, uint32_t width, uint32_t height, uint32_t arrayCount,	DXGI_FORMAT format);

	// Get pre-created CPU-visible descriptor handles
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetSRV(void) const { return m_srvHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetRTV(void) const { return m_rtvHandle; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetUAV(void) const { return m_uavHandle[0]; }

	void SetClearColor(Color clearColor) { m_clearColor = clearColor; }

	void SetMsaaMode(uint32_t numColorSamples, uint32_t numCoverageSamples)
	{
		assert(numCoverageSamples >= numColorSamples);
		m_fragmentCount = numColorSamples;
		m_sampleCount = numCoverageSamples;
	}

	Color GetClearColor() const { return m_clearColor; }

	// This will work for all texture sizes, but it's recommended for speed and quality
	// that you use dimensions with powers of two (but not necessarily square.)  Pass
	// 0 for ArrayCount to reserve space for mips at creation time.
	// TODO
	//void GenerateMipMaps(CommandContext& context);

protected:
	D3D12_RESOURCE_FLAGS CombineResourceFlags() const
	{
		D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

		if (flags == D3D12_RESOURCE_FLAG_NONE && m_fragmentCount == 1)
		{
			flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		}

		return D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | flags;
	}

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

	void CreateDerivedViews(DXGI_FORMAT format, uint32_t arraySize, uint32_t numMips = 1);

protected:
	Color m_clearColor;
	D3D12_CPU_DESCRIPTOR_HANDLE m_srvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_rtvHandle;
	D3D12_CPU_DESCRIPTOR_HANDLE m_uavHandle[12];
	uint32_t m_numMipMaps{ 0 }; // number of texture sublevels
	uint32_t m_fragmentCount{ 1 };
	uint32_t m_sampleCount{ 1 };
};

} // namespace Kodiak