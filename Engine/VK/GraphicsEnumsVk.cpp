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

#include "GraphicsEnumsVk.h"


using namespace Kodiak;
using namespace std;


namespace Kodiak
{
const uint32_t Defaults::DepthBias = 0;
const float Defaults::SlopeScaledDepthBias = 0.0f;
const float Defaults::DepthBiasClamp = 0.0f;
const byte Defaults::StencilReadMask = 0xFF;
const byte Defaults::StencilWriteMask = 0xFF;
const float Defaults::Float32Max = FLT_MAX;

const uint32_t Limits::MaxTextureDimension1D = 16384;
const uint32_t Limits::MaxTextureDimension2D = 16384;
const uint32_t Limits::MaxTextureDimension3D = 2048;
const uint32_t Limits::MaxTextureDimensionCube = 16384;
const uint32_t Limits::MaxTexture1DArrayElements = 2048;
const uint32_t Limits::MaxTexture2DArrayElements = 2048;
const uint32_t Limits::MaxTextureMipLevels = 15;
} // namespace Kodiak


Format Kodiak::MapVulkanFormatToEngine(VkFormat format)
{
	static bool initialized = false;
	static array<Format, VK_FORMAT_RANGE_SIZE> remapTable;

	if (!initialized)
	{
		for (uint32_t i = 0; i < VK_FORMAT_RANGE_SIZE; ++i)
		{
			remapTable[i] = Format::Unknown;
		}

		// Map known types
		remapTable[VK_FORMAT_B4G4R4A4_UNORM_PACK16] = Format::B4G4R4A4_UNorm;
		remapTable[VK_FORMAT_B5G6R5_UNORM_PACK16] = Format::B5G6R5_UNorm;
		remapTable[VK_FORMAT_B5G5R5A1_UNORM_PACK16] = Format::B5G5R5A1_UNorm;
		remapTable[VK_FORMAT_B8G8R8A8_UNORM] = Format::B8G8R8A8_UNorm;
		remapTable[VK_FORMAT_R8_UNORM] = Format::R8_UNorm;
		remapTable[VK_FORMAT_R8_SNORM] = Format::R8_SNorm;
		remapTable[VK_FORMAT_R8_UINT] = Format::R8_UInt;
		remapTable[VK_FORMAT_R8_SINT] = Format::R8_SInt;
		remapTable[VK_FORMAT_R8G8_UNORM] = Format::R8G8_UNorm;
		remapTable[VK_FORMAT_R8G8_SNORM] = Format::R8G8_SNorm;
		remapTable[VK_FORMAT_R8G8_UINT] = Format::R8G8_UInt;
		remapTable[VK_FORMAT_R8G8_SINT] = Format::R8G8_SInt;
		remapTable[VK_FORMAT_R8G8B8A8_UNORM] = Format::R8G8B8A8_UNorm;
		remapTable[VK_FORMAT_R8G8B8A8_SNORM] = Format::R8G8B8A8_SNorm;
		remapTable[VK_FORMAT_R8G8B8A8_UINT] = Format::R8G8B8A8_UInt;
		remapTable[VK_FORMAT_R8G8B8A8_SINT] = Format::R8G8B8A8_SInt;
		remapTable[VK_FORMAT_R16_UNORM] = Format::R16_UNorm;
		remapTable[VK_FORMAT_R16_SNORM] = Format::R16_SNorm;
		remapTable[VK_FORMAT_R16_UINT] = Format::R16_UInt;
		remapTable[VK_FORMAT_R16_SINT] = Format::R16_SInt;
		remapTable[VK_FORMAT_R16_SFLOAT] = Format::R16_Float;
		remapTable[VK_FORMAT_R16G16_UNORM] = Format::R16G16_UNorm;
		remapTable[VK_FORMAT_R16G16_SNORM] = Format::R16G16_SNorm;
		remapTable[VK_FORMAT_R16G16_UINT] = Format::R16G16_UInt;
		remapTable[VK_FORMAT_R16G16_SINT] = Format::R16G16_SInt;
		remapTable[VK_FORMAT_R16G16_SFLOAT] = Format::R16G16_Float;
		remapTable[VK_FORMAT_R16G16B16A16_UNORM] = Format::R16G16B16A16_UNorm;
		remapTable[VK_FORMAT_R16G16B16A16_SNORM] = Format::R16G16B16A16_SNorm;
		remapTable[VK_FORMAT_R16G16B16A16_UINT] = Format::R16G16B16A16_UInt;
		remapTable[VK_FORMAT_R16G16B16A16_SINT] = Format::R16G16B16A16_SInt;
		remapTable[VK_FORMAT_R16G16B16A16_SFLOAT] = Format::R16G16B16A16_Float;
		remapTable[VK_FORMAT_R32_UINT] = Format::R32_UInt;
		remapTable[VK_FORMAT_R32_SINT] = Format::R32_SInt;
		remapTable[VK_FORMAT_R32_SFLOAT] = Format::R32_Float;
		remapTable[VK_FORMAT_R32G32_UINT] = Format::R32G32_UInt;
		remapTable[VK_FORMAT_R32G32_SINT] = Format::R32G32_SInt;
		remapTable[VK_FORMAT_R32G32_SFLOAT] = Format::R32G32_Float;
		remapTable[VK_FORMAT_R32G32B32_UINT] = Format::R32G32B32_UInt;
		remapTable[VK_FORMAT_R32G32B32_SINT] = Format::R32G32B32_SInt;
		remapTable[VK_FORMAT_R32G32B32_SFLOAT] = Format::R32G32B32_Float;
		remapTable[VK_FORMAT_R32G32B32A32_UINT] = Format::R32G32B32A32_UInt;
		remapTable[VK_FORMAT_R32G32B32A32_SINT] = Format::R32G32B32A32_SInt;
		remapTable[VK_FORMAT_R32G32B32A32_SFLOAT] = Format::R32G32B32A32_Float;
		remapTable[VK_FORMAT_B10G11R11_UFLOAT_PACK32] = Format::R11G11B10_Float;
		remapTable[VK_FORMAT_A2R10G10B10_UNORM_PACK32] = Format::R10G10B10A2_UNorm;

		remapTable[VK_FORMAT_D16_UNORM] = Format::D16_UNorm;
		remapTable[VK_FORMAT_D24_UNORM_S8_UINT] = Format::D24S8;
		remapTable[VK_FORMAT_D32_SFLOAT] = Format::D32_Float;
		remapTable[VK_FORMAT_D32_SFLOAT_S8_UINT] = Format::D32_Float_S8_UInt;

		remapTable[VK_FORMAT_BC1_RGBA_UNORM_BLOCK] = Format::BC1_UNorm;
		remapTable[VK_FORMAT_BC1_RGBA_SRGB_BLOCK] = Format::BC1_UNorm_SRGB;
		remapTable[VK_FORMAT_BC2_UNORM_BLOCK] = Format::BC2_UNorm;
		remapTable[VK_FORMAT_BC2_SRGB_BLOCK] = Format::BC2_UNorm_SRGB;
		remapTable[VK_FORMAT_BC3_UNORM_BLOCK] = Format::BC3_UNorm;
		remapTable[VK_FORMAT_BC3_SRGB_BLOCK] = Format::BC3_UNorm_SRGB;
		remapTable[VK_FORMAT_BC4_UNORM_BLOCK] = Format::BC4_UNorm;
		remapTable[VK_FORMAT_BC4_SNORM_BLOCK] = Format::BC4_SNorm;
		remapTable[VK_FORMAT_BC5_UNORM_BLOCK] = Format::BC5_UNorm;
		remapTable[VK_FORMAT_BC5_SNORM_BLOCK] = Format::BC5_SNorm;
		remapTable[VK_FORMAT_BC6H_SFLOAT_BLOCK] = Format::BC6H_Float;
		remapTable[VK_FORMAT_BC6H_UFLOAT_BLOCK] = Format::BC6H_UFloat;
		remapTable[VK_FORMAT_BC7_UNORM_BLOCK] = Format::BC7_UNorm;
		remapTable[VK_FORMAT_BC7_SRGB_BLOCK] = Format::BC7_UNorm_SRGB;

		initialized = true;
	}

	return remapTable[format];
}


VkPrimitiveTopology Kodiak::MapEnginePrimitiveTopologyToVulkan(PrimitiveTopology topology)
{
	switch (topology)
	{
	case PrimitiveTopology::PointList: return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
	case PrimitiveTopology::LineList: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
	case PrimitiveTopology::LineStrip: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
	case PrimitiveTopology::TriangleList: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	case PrimitiveTopology::TriangleStrip: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
	case PrimitiveTopology::LineListWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
	case PrimitiveTopology::LineStripWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
	case PrimitiveTopology::TriangleListWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
	case PrimitiveTopology::TriangleStripWithAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
	default:
		return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
	}
}


uint32_t Kodiak::GetControlPointCount(PrimitiveTopology topology)
{
	switch (topology)
	{
	case PrimitiveTopology::Patch_1_ControlPoint:	return 1;
	case PrimitiveTopology::Patch_2_ControlPoint:	return 2;
	case PrimitiveTopology::Patch_3_ControlPoint:	return 3;
	case PrimitiveTopology::Patch_4_ControlPoint:	return 4;
	case PrimitiveTopology::Patch_5_ControlPoint:	return 5;
	case PrimitiveTopology::Patch_6_ControlPoint:	return 6;
	case PrimitiveTopology::Patch_7_ControlPoint:	return 7;
	case PrimitiveTopology::Patch_8_ControlPoint:	return 8;
	case PrimitiveTopology::Patch_9_ControlPoint:	return 9;
	case PrimitiveTopology::Patch_10_ControlPoint:	return 10;
	case PrimitiveTopology::Patch_11_ControlPoint:	return 11;
	case PrimitiveTopology::Patch_12_ControlPoint:	return 12;
	case PrimitiveTopology::Patch_13_ControlPoint:	return 13;
	case PrimitiveTopology::Patch_14_ControlPoint:	return 14;
	case PrimitiveTopology::Patch_15_ControlPoint:	return 15;
	case PrimitiveTopology::Patch_16_ControlPoint:	return 16;
	case PrimitiveTopology::Patch_17_ControlPoint:	return 17;
	case PrimitiveTopology::Patch_18_ControlPoint:	return 18;
	case PrimitiveTopology::Patch_19_ControlPoint:	return 19;
	case PrimitiveTopology::Patch_20_ControlPoint:	return 20;
	case PrimitiveTopology::Patch_21_ControlPoint:	return 21;
	case PrimitiveTopology::Patch_22_ControlPoint:	return 22;
	case PrimitiveTopology::Patch_23_ControlPoint:	return 23;
	case PrimitiveTopology::Patch_24_ControlPoint:	return 24;
	case PrimitiveTopology::Patch_25_ControlPoint:	return 25;
	case PrimitiveTopology::Patch_26_ControlPoint:	return 26;
	case PrimitiveTopology::Patch_27_ControlPoint:	return 27;
	case PrimitiveTopology::Patch_28_ControlPoint:	return 28;
	case PrimitiveTopology::Patch_29_ControlPoint:	return 29;
	case PrimitiveTopology::Patch_30_ControlPoint:	return 30;
	case PrimitiveTopology::Patch_31_ControlPoint:	return 31;
	case PrimitiveTopology::Patch_32_ControlPoint:	return 32;
	default:
		return 0;
	}
}