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

} // namespace Kodiak