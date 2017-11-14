//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#pragma once

namespace Kodiak
{

HRESULT __cdecl CreateKTXTextureFromMemory(ID3D12Device* d3dDevice,
	const uint8_t* ktxData,
	size_t ktxDataSize,
	size_t maxsize,
	bool forceSRGB,
	ID3D12Resource** texture,
	D3D12_CPU_DESCRIPTOR_HANDLE textureView
);


HRESULT __cdecl CreateKTXTextureFromFile(ID3D12Device* d3dDevice,
	const char* szFileName,
	size_t maxsize,
	bool forceSRGB,
	ID3D12Resource** texture,
	D3D12_CPU_DESCRIPTOR_HANDLE textureView
);

} // namespace Kodiak