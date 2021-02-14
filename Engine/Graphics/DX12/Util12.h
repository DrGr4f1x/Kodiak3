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

D3D12_UAV_DIMENSION GetUAVDimension(ResourceType type);
D3D12_SRV_DIMENSION GetSRVDimension(ResourceType type);
D3D12_RESOURCE_DIMENSION GetResourceDimension(ResourceType type);

DXGI_FORMAT GetBaseFormat(DXGI_FORMAT format);
DXGI_FORMAT GetUAVFormat(DXGI_FORMAT format);
DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format);
DXGI_FORMAT GetDepthFormat(DXGI_FORMAT format);
DXGI_FORMAT GetStencilFormat(DXGI_FORMAT format);

size_t BytesPerPixel(DXGI_FORMAT format);

D3D12_RESOURCE_DESC DescribeTex2D(uint32_t width, uint32_t height, uint32_t depthOrArraySize,
	uint32_t numMips, uint32_t numSamples, Format format, uint32_t flags);

void CreateTextureResource(const std::string& name, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_CLEAR_VALUE clearValue, ID3D12Resource** ppResource);

D3D12_RESOURCE_STATES GetResourceState(ResourceState state);
D3D12_QUERY_HEAP_TYPE GetQueryHeapType(QueryHeapType type);
D3D12_QUERY_TYPE GetQueryType(QueryType type);

} // namespace Kodiak