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

enum DDS_ALPHA_MODE
{
	DDS_ALPHA_MODE_UNKNOWN = 0,
	DDS_ALPHA_MODE_STRAIGHT = 1,
	DDS_ALPHA_MODE_PREMULTIPLIED = 2,
	DDS_ALPHA_MODE_OPAQUE = 3,
	DDS_ALPHA_MODE_CUSTOM = 4,
};


HRESULT __cdecl CreateDDSTextureFromMemory(ID3D12Device* d3dDevice,
	const uint8_t* ddsData,
	size_t ddsDataSize,
	size_t maxsize,
	Format format,
	bool forceSRGB,
	ID3D12Resource** texture,
	D3D12_CPU_DESCRIPTOR_HANDLE textureView,
	DDS_ALPHA_MODE* alphaMode = nullptr
);


HRESULT __cdecl CreateDDSTextureFromFile(ID3D12Device* d3dDevice,
	const char* szFileName,
	size_t maxsize,
	Format format,
	bool forceSRGB,
	ID3D12Resource** texture,
	D3D12_CPU_DESCRIPTOR_HANDLE textureView,
	DDS_ALPHA_MODE* alphaMode = nullptr
);

} // namespace Kodiak