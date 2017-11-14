//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author: David Elder
//

#include "Stdafx.h"

#include "KTXTextureLoader12.h"


using namespace Kodiak;
using namespace std;


namespace
{

unsigned char const FOURCC_KTX10[] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x31, 0x31, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };
unsigned char const FOURCC_KTX20[] = { 0xAB, 0x4B, 0x54, 0x58, 0x20, 0x32, 0x30, 0xBB, 0x0D, 0x0A, 0x1A, 0x0A };

struct KTXHeader10
{
	uint32_t endianness;
	uint32_t glType;
	uint32_t glTypeSize;
	uint32_t glFormat;
	uint32_t glInternalFormat;
	uint32_t glBaseInternalFormat;
	uint32_t pixelWidth;
	uint32_t pixelHeight;
	uint32_t pixelDepth;
	uint32_t numberOfArrayElements;
	uint32_t numberOfFaces;
	uint32_t numberOfMipmapLevels;
	uint32_t bytesOfKeyValueData;
};

} // anonymous namespace


HRESULT Kodiak::CreateKTXTextureFromMemory(ID3D12Device* d3dDevice,
	const uint8_t* ktxData,
	size_t ktxDataSize,
	size_t maxsize,
	bool forceSRGB,
	ID3D12Resource** texture,
	D3D12_CPU_DESCRIPTOR_HANDLE textureView
)
{
	HRESULT res = S_OK;

	return res;
}


HRESULT Kodiak::CreateKTXTextureFromFile(ID3D12Device* d3dDevice,
	const char* szFileName,
	size_t maxsize,
	bool forceSRGB,
	ID3D12Resource** texture,
	D3D12_CPU_DESCRIPTOR_HANDLE textureView
)
{
	HRESULT res = S_OK;

	return res;
}