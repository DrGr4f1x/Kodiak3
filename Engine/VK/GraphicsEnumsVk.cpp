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

		initialized = true;
	}

	return remapTable[format];
}