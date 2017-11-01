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

#include "GpuResource12.h"

namespace Kodiak
{

class PixelBuffer : public GpuResource
{
public:
	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	uint32_t GetDepth() const { return m_arraySize; }
	Format GetFormat() const { return m_format; }

protected:
	D3D12_RESOURCE_DESC DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize, uint32_t numMips, Format format, uint32_t flags);

	void AssociateWithResource(const std::string& name, ID3D12Resource* resource, D3D12_RESOURCE_STATES currentState);

	void CreateTextureResource(const std::string& name, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_CLEAR_VALUE clearValue);

	static DXGI_FORMAT GetBaseFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetUAVFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetDepthFormat(DXGI_FORMAT format);
	static DXGI_FORMAT GetStencilFormat(DXGI_FORMAT format);
	static size_t BytesPerPixel(DXGI_FORMAT format);

protected:
	uint32_t m_width{ 0 };
	uint32_t m_height{ 0 };
	uint32_t m_arraySize{ 0 };
	Format m_format{ Format::Unknown };
};

} // namespace Kodiak