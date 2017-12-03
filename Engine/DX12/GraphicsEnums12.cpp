//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Stdafx.h"

#include "GraphicsEnums12.h"


using namespace Kodiak;
using namespace std;


namespace Kodiak
{
const uint32_t Defaults::DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
const float Defaults::SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
const float Defaults::DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
const byte Defaults::StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
const byte Defaults::StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
const float Defaults::Float32Max = D3D12_FLOAT32_MAX;

const uint32_t Limits::MaxTextureDimension1D = D3D12_REQ_TEXTURE1D_U_DIMENSION;
const uint32_t Limits::MaxTextureDimension2D = D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION;
const uint32_t Limits::MaxTextureDimension3D = D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION;
const uint32_t Limits::MaxTextureDimensionCube = D3D12_REQ_TEXTURECUBE_DIMENSION;
const uint32_t Limits::MaxTexture1DArrayElements = D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION;
const uint32_t Limits::MaxTexture2DArrayElements = D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION;
const uint32_t Limits::MaxTextureMipLevels = D3D12_REQ_MIP_LEVELS;
} // namespace Kodiak


Format Kodiak::MapDXGIFormatToEngine(DXGI_FORMAT format)
{
	static bool initialized = false;
	static array<Format, DXGI_FORMAT_V408 + 1> remapTable;

	if (!initialized)
	{
		for (uint32_t i = 0; i < DXGI_FORMAT_V408 + 1; ++i)
		{
			remapTable[i] = Format::Unknown;
		}

		remapTable[DXGI_FORMAT_B4G4R4A4_UNORM] = Format::B4G4R4A4_UNorm;
		remapTable[DXGI_FORMAT_B5G6R5_UNORM] = Format::B5G6R5_UNorm;
		remapTable[DXGI_FORMAT_B5G5R5A1_UNORM] = Format::B5G5R5A1_UNorm;
		remapTable[DXGI_FORMAT_B8G8R8A8_UNORM] = Format::B8G8R8A8_UNorm;
		remapTable[DXGI_FORMAT_R8_UNORM] = Format::R8_UNorm;
		remapTable[DXGI_FORMAT_R8_SNORM] = Format::R8_SNorm;
		remapTable[DXGI_FORMAT_R8_UINT] = Format::R8_UInt;
		remapTable[DXGI_FORMAT_R8_SINT] = Format::R8_SInt;
		remapTable[DXGI_FORMAT_R8G8_UNORM] = Format::R8G8_UNorm;
		remapTable[DXGI_FORMAT_R8G8_SNORM] = Format::R8G8_SNorm;
		remapTable[DXGI_FORMAT_R8G8_UINT] = Format::R8G8_UInt;
		remapTable[DXGI_FORMAT_R8G8_SINT] = Format::R8G8_SInt;
		remapTable[DXGI_FORMAT_R8G8B8A8_UNORM] = Format::R8G8B8A8_UNorm;
		remapTable[DXGI_FORMAT_R8G8B8A8_SNORM] = Format::R8G8B8A8_SNorm;
		remapTable[DXGI_FORMAT_R8G8B8A8_UINT] = Format::R8G8B8A8_UInt;
		remapTable[DXGI_FORMAT_R8G8B8A8_SINT] = Format::R8G8B8A8_SInt;
		remapTable[DXGI_FORMAT_R16_UNORM] = Format::R16_UNorm;
		remapTable[DXGI_FORMAT_R16_SNORM] = Format::R16_SNorm;
		remapTable[DXGI_FORMAT_R16_UINT] = Format::R16_UInt;
		remapTable[DXGI_FORMAT_R16_SINT] = Format::R16_SInt;
		remapTable[DXGI_FORMAT_R16_FLOAT] = Format::R16_Float;
		remapTable[DXGI_FORMAT_R16G16_UNORM] = Format::R16G16_UNorm;
		remapTable[DXGI_FORMAT_R16G16_SNORM] = Format::R16G16_SNorm;
		remapTable[DXGI_FORMAT_R16G16_UINT] = Format::R16G16_UInt;
		remapTable[DXGI_FORMAT_R16G16_SINT] = Format::R16G16_SInt;
		remapTable[DXGI_FORMAT_R16G16_FLOAT] = Format::R16G16_Float;
		remapTable[DXGI_FORMAT_R16G16B16A16_UNORM] = Format::R16G16B16A16_UNorm;
		remapTable[DXGI_FORMAT_R16G16B16A16_SNORM] = Format::R16G16B16A16_SNorm;
		remapTable[DXGI_FORMAT_R16G16B16A16_UINT] = Format::R16G16B16A16_UInt;
		remapTable[DXGI_FORMAT_R16G16B16A16_SINT] = Format::R16G16B16A16_SInt;
		remapTable[DXGI_FORMAT_R16G16B16A16_FLOAT] = Format::R16G16B16A16_Float;
		remapTable[DXGI_FORMAT_R32_UINT] = Format::R32_UInt;
		remapTable[DXGI_FORMAT_R32_SINT] = Format::R32_SInt;
		remapTable[DXGI_FORMAT_R32_FLOAT] = Format::R32_Float;
		remapTable[DXGI_FORMAT_R32G32_UINT] = Format::R32G32_UInt;
		remapTable[DXGI_FORMAT_R32G32_SINT] = Format::R32G32_SInt;
		remapTable[DXGI_FORMAT_R32G32_FLOAT] = Format::R32G32_Float;
		remapTable[DXGI_FORMAT_R32G32B32_UINT] = Format::R32G32B32_UInt;
		remapTable[DXGI_FORMAT_R32G32B32_SINT] = Format::R32G32B32_SInt;
		remapTable[DXGI_FORMAT_R32G32B32_FLOAT] = Format::R32G32B32_Float;
		remapTable[DXGI_FORMAT_R32G32B32A32_UINT] = Format::R32G32B32A32_UInt;
		remapTable[DXGI_FORMAT_R32G32B32A32_SINT] = Format::R32G32B32A32_SInt;
		remapTable[DXGI_FORMAT_R32G32B32A32_FLOAT] = Format::R32G32B32A32_Float;
		remapTable[DXGI_FORMAT_R11G11B10_FLOAT] = Format::R11G11B10_Float;
		remapTable[DXGI_FORMAT_R10G10B10A2_UNORM] = Format::R10G10B10A2_UNorm;

		remapTable[DXGI_FORMAT_D16_UNORM] = Format::D16_UNorm;
		remapTable[DXGI_FORMAT_D24_UNORM_S8_UINT] = Format::D24S8;
		remapTable[DXGI_FORMAT_D32_FLOAT] = Format::D32_Float;
		remapTable[DXGI_FORMAT_D32_FLOAT_S8X24_UINT] = Format::D32_Float_S8_UInt;

		remapTable[DXGI_FORMAT_BC1_UNORM] = Format::BC1_UNorm;
		remapTable[DXGI_FORMAT_BC1_UNORM_SRGB] = Format::BC1_UNorm_SRGB;
		remapTable[DXGI_FORMAT_BC2_UNORM] = Format::BC2_UNorm;
		remapTable[DXGI_FORMAT_BC2_UNORM_SRGB] = Format::BC2_UNorm_SRGB;
		remapTable[DXGI_FORMAT_BC3_UNORM] = Format::BC3_UNorm;
		remapTable[DXGI_FORMAT_BC3_UNORM_SRGB] = Format::BC3_UNorm_SRGB;
		remapTable[DXGI_FORMAT_BC4_UNORM] = Format::BC4_UNorm;
		remapTable[DXGI_FORMAT_BC4_SNORM] = Format::BC4_SNorm;
		remapTable[DXGI_FORMAT_BC5_UNORM] = Format::BC5_UNorm;
		remapTable[DXGI_FORMAT_BC5_SNORM] = Format::BC5_SNorm;
		remapTable[DXGI_FORMAT_BC6H_SF16] = Format::BC6H_Float;
		remapTable[DXGI_FORMAT_BC6H_UF16] = Format::BC6H_UFloat;
		remapTable[DXGI_FORMAT_BC7_UNORM] = Format::BC7_UNorm;
		remapTable[DXGI_FORMAT_BC7_UNORM_SRGB] = Format::BC7_UNorm_SRGB;

		initialized = true;
	}

	return remapTable[format];
}


D3D12_PRIMITIVE_TOPOLOGY_TYPE Kodiak::MapPrimitiveTopologyToD3DType(PrimitiveTopology topology)
{
	switch (topology)
	{
	case PrimitiveTopology::PointList:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;

	case PrimitiveTopology::LineList:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case PrimitiveTopology::LineStrip:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

	case PrimitiveTopology::TriangleList:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case PrimitiveTopology::TriangleStrip:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	case PrimitiveTopology::LineListWithAdjacency:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
	case PrimitiveTopology::LineStripWithAdjacency:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;

	case PrimitiveTopology::TriangleListWithAdjacency:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	case PrimitiveTopology::TriangleStripWithAdjacency:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;

	default:
		return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
	}
}