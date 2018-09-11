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

#if defined(DX12)
#include "DX12\GraphicsEnums12.h"
#elif defined(VK)
#include "VK\GraphicsEnumsVk.h"
#else
#error "No graphics API defined"
#endif

namespace Kodiak
{

struct Defaults
{
	static const uint32_t DepthBias;
	static const float DepthBiasClamp;
	static const float SlopeScaledDepthBias;
	static const byte StencilReadMask;
	static const byte StencilWriteMask;
	static const float Float32Max;
};


struct Limits
{
	static const uint32_t MaxTextureDimension1D;
	static const uint32_t MaxTextureDimension2D;
	static const uint32_t MaxTextureDimension3D;
	static const uint32_t MaxTextureDimensionCube;
	static const uint32_t MaxTexture1DArrayElements;
	static const uint32_t MaxTexture2DArrayElements;
	static const uint32_t MaxTextureMipLevels;
};


enum class ResourceState
{
	Undefined,
	Common,
	VertexBuffer,
	IndexBuffer,
	ConstantBuffer,
	RenderTarget,
	UnorderedAccess,
	DepthWrite,
	DepthRead,
	NonPixelShaderResource,
	PixelShaderResource,
	ShaderResource,
	StreamOut,
	IndirectArgument,
	CopyDest,
	CopySource,
	ResolveDest,
	ResolveSource,
	GenericRead,
	Present,
	Predication,
	PreInitialized,
};


enum class QueryHeapType
{
	Occlusion,
	Timestamp,
	PipelineStats
};


enum class QueryType
{
	Occlusion,
	Timestamp,
	PipelineStats
};


inline uint32_t BitsPerPixel(Format format)
{
	switch (format)
	{
	case Format::R32G32B32A32_Float:
	case Format::R32G32B32A32_SInt:
	case Format::R32G32B32A32_UInt:
		return 128;

	case Format::R32G32B32_Float:
	case Format::R32G32B32_SInt:
	case Format::R32G32B32_UInt:
		return 96;

	case Format::R16G16B16A16_Float:
	case Format::R16G16B16A16_SInt:
	case Format::R16G16B16A16_SNorm:
	case Format::R16G16B16A16_UInt:
	case Format::R16G16B16A16_UNorm:
	case Format::R32G32_Float:
	case Format::R32G32_SInt:
	case Format::R32G32_UInt:
	case Format::D32_Float_S8_UInt:
		return 64;

	case Format::R10G10B10A2_UNorm:
	case Format::R11G11B10_Float:
	case Format::B8G8R8A8_UNorm:
	case Format::R8G8B8A8_SInt:
	case Format::R8G8B8A8_SNorm:
	case Format::R8G8B8A8_UInt:
	case Format::R8G8B8A8_UNorm:
	case Format::R8G8B8A8_UNorm_SRGB:
	case Format::R16G16_Float:
	case Format::R16G16_SInt:
	case Format::R16G16_SNorm:
	case Format::R16G16_UInt:
	case Format::R16G16_UNorm:
	case Format::R32_Float:
	case Format::R32_SInt:
	case Format::R32_UInt:
	case Format::D32_Float:
	case Format::D24S8:
		return 32;

	case Format::R8G8_SInt:
	case Format::R8G8_SNorm:
	case Format::R8G8_UInt:
	case Format::R8G8_UNorm:
	case Format::R16_Float:
	case Format::R16_SInt:
	case Format::R16_SNorm:
	case Format::R16_UInt:
	case Format::R16_UNorm:
	case Format::B4G4R4A4_UNorm:
	case Format::B5G5R5A1_UNorm:
	case Format::B5G6R5_UNorm:
	case Format::D16_UNorm:
		return 16;

	case Format::R8_SInt:
	case Format::R8_SNorm:
	case Format::R8_UInt:
	case Format::R8_UNorm:
		return 8;

	case Format::BC1_UNorm:
	case Format::BC1_UNorm_SRGB:
	case Format::BC4_UNorm:
	case Format::BC4_SNorm:
		return 4;

	case Format::BC2_UNorm:
	case Format::BC2_UNorm_SRGB:
	case Format::BC3_UNorm:
	case Format::BC3_UNorm_SRGB:
	case Format::BC5_UNorm:
	case Format::BC5_SNorm:
	case Format::BC6H_Float:
	case Format::BC6H_UFloat:
	case Format::BC7_UNorm:
	case Format::BC7_UNorm_SRGB:
		return 8;

	default:
		return 0;
	}
}


inline uint32_t BlockSize(Format format)
{
	switch (format)
	{
	case Format::R32G32B32A32_Float:
	case Format::R32G32B32A32_SInt:
	case Format::R32G32B32A32_UInt:
		return 16;

	case Format::R32G32B32_Float:
	case Format::R32G32B32_SInt:
	case Format::R32G32B32_UInt:
		return 12;

	case Format::R16G16B16A16_Float:
	case Format::R16G16B16A16_SInt:
	case Format::R16G16B16A16_SNorm:
	case Format::R16G16B16A16_UInt:
	case Format::R16G16B16A16_UNorm:
	case Format::R32G32_Float:
	case Format::R32G32_SInt:
	case Format::R32G32_UInt:
		return 8;

	case Format::D32_Float_S8_UInt:
		return 5;

	case Format::R10G10B10A2_UNorm:
	case Format::R11G11B10_Float:
	case Format::B8G8R8A8_UNorm:
	case Format::R8G8B8A8_SInt:
	case Format::R8G8B8A8_SNorm:
	case Format::R8G8B8A8_UInt:
	case Format::R8G8B8A8_UNorm:
	case Format::R8G8B8A8_UNorm_SRGB:
	case Format::R16G16_Float:
	case Format::R16G16_SInt:
	case Format::R16G16_SNorm:
	case Format::R16G16_UInt:
	case Format::R16G16_UNorm:
	case Format::R32_Float:
	case Format::R32_SInt:
	case Format::R32_UInt:
	case Format::D32_Float:
	case Format::D24S8:
		return 4;

	case Format::R8G8_SInt:
	case Format::R8G8_SNorm:
	case Format::R8G8_UInt:
	case Format::R8G8_UNorm:
	case Format::R16_Float:
	case Format::R16_SInt:
	case Format::R16_SNorm:
	case Format::R16_UInt:
	case Format::R16_UNorm:
	case Format::B4G4R4A4_UNorm:
	case Format::B5G5R5A1_UNorm:
	case Format::B5G6R5_UNorm:
	case Format::D16_UNorm:
		return 2;

	case Format::R8_SInt:
	case Format::R8_SNorm:
	case Format::R8_UInt:
	case Format::R8_UNorm:
		return 1;

	case Format::BC1_UNorm:
	case Format::BC1_UNorm_SRGB:
	case Format::BC4_UNorm:
	case Format::BC4_SNorm:
		return 8;

	case Format::BC2_UNorm:
	case Format::BC2_UNorm_SRGB:
	case Format::BC3_UNorm:
	case Format::BC3_UNorm_SRGB:
	case Format::BC5_UNorm:
	case Format::BC5_SNorm:
	case Format::BC6H_Float:
	case Format::BC6H_UFloat:
	case Format::BC7_UNorm:
	case Format::BC7_UNorm_SRGB:
		return 16;

	default:
		return 0;
	}
}


inline Format MakeSRGB(Format format)
{
	switch (format)
	{
	case Format::R8G8B8A8_UNorm:
		return Format::R8G8B8A8_UNorm_SRGB;
	}

	return format;
}


inline bool IsDepthFormat(Format format)
{
	return 
		format == Format::D32_Float_S8_UInt ||
		format == Format::D32_Float ||
		format == Format::D24S8 ||
		format == Format::D16_UNorm;
}


inline bool IsStencilFormat(Format format)
{
	return format == Format::D24S8 || format == Format::D32_Float_S8_UInt;
}


inline bool IsDepthStencilFormat(Format format)
{
	return IsDepthFormat(format) || IsStencilFormat(format);
}


enum class ResourceType
{
	Unknown,
	Texture1D,
	Texture1D_Array,
	Texture2D,
	Texture2D_Array,
	Texture2DMS,
	Texture2DMS_Array,
	TextureCube,
	TextureCube_Array,
	Texture3D,
	GenericBuffer,
	IndexBuffer,
	VertexBuffer,
	StructuredBuffer,
	TypedBuffer
};


inline bool IsTextureResource(ResourceType type)
{
	return (type >= ResourceType::Texture1D) && (type <= ResourceType::Texture3D);
}


inline bool IsBufferResource(ResourceType type)
{
	return (type > ResourceType::Texture3D);
}

} // namespace Kodiak