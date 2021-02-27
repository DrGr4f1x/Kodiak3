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

#include "Util12.h"

#include "Graphics\GraphicsDevice.h"

using namespace std;


namespace Kodiak
{

D3D12_UAV_DIMENSION GetUAVDimension(ResourceType type)
{
	switch (type)
	{
	case ResourceType::Texture1D:
		return D3D12_UAV_DIMENSION_TEXTURE1D;

	case ResourceType::Texture1D_Array:
		return D3D12_UAV_DIMENSION_TEXTURE1D;

	case ResourceType::Texture2D:
	case ResourceType::Texture2DMS:
		return D3D12_UAV_DIMENSION_TEXTURE2D;

	case ResourceType::Texture2D_Array:
	case ResourceType::Texture2DMS_Array:
	case ResourceType::TextureCube:
		return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;

	case ResourceType::IndexBuffer:
	case ResourceType::VertexBuffer:
	case ResourceType::ConstantBuffer:
	case ResourceType::ByteAddressBuffer:
	case ResourceType::IndirectArgsBuffer:
	case ResourceType::StructuredBuffer:
	case ResourceType::TypedBuffer:
	case ResourceType::ReadbackBuffer:
		return D3D12_UAV_DIMENSION_BUFFER;

	default:
		assert(false);
		return D3D12_UAV_DIMENSION_UNKNOWN;
	}
}


D3D12_SRV_DIMENSION GetSRVDimension(ResourceType type)
{
	switch (type)
	{
	case ResourceType::Texture1D:
		return D3D12_SRV_DIMENSION_TEXTURE1D;
	case ResourceType::Texture1D_Array:
		return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
	case ResourceType::Texture2D:
		return D3D12_SRV_DIMENSION_TEXTURE2D;
	case ResourceType::Texture2D_Array:
		return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
	case ResourceType::Texture2DMS:
		return D3D12_SRV_DIMENSION_TEXTURE2DMS;
	case ResourceType::Texture2DMS_Array:
		return D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
	case ResourceType::Texture3D:
		return D3D12_SRV_DIMENSION_TEXTURE3D;
	case ResourceType::TextureCube:
		return D3D12_SRV_DIMENSION_TEXTURECUBE;
	case ResourceType::TextureCube_Array:
		return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;

	case ResourceType::IndexBuffer:
	case ResourceType::VertexBuffer:
	case ResourceType::ConstantBuffer:
	case ResourceType::ByteAddressBuffer:
	case ResourceType::IndirectArgsBuffer:
	case ResourceType::StructuredBuffer:
	case ResourceType::TypedBuffer:
	case ResourceType::ReadbackBuffer:
		return D3D12_SRV_DIMENSION_BUFFER;

	default:
		assert(false);
		return D3D12_SRV_DIMENSION_UNKNOWN;
	}
}


D3D12_RESOURCE_DIMENSION GetResourceDimension(ResourceType type)
{
	switch (type)
	{
	case ResourceType::Texture1D:
	case ResourceType::Texture1D_Array:
		return D3D12_RESOURCE_DIMENSION_TEXTURE1D;

	case ResourceType::Texture2D:
	case ResourceType::Texture2D_Array:
	case ResourceType::Texture2DMS:
	case ResourceType::Texture2DMS_Array:
	case ResourceType::TextureCube:
	case ResourceType::TextureCube_Array:
		return D3D12_RESOURCE_DIMENSION_TEXTURE2D;

	case ResourceType::Texture3D:
		return D3D12_RESOURCE_DIMENSION_TEXTURE3D;

	case ResourceType::IndexBuffer:
	case ResourceType::VertexBuffer:
	case ResourceType::ConstantBuffer:
	case ResourceType::ByteAddressBuffer:
	case ResourceType::IndirectArgsBuffer:
	case ResourceType::StructuredBuffer:
	case ResourceType::TypedBuffer:
	case ResourceType::ReadbackBuffer:
		return D3D12_RESOURCE_DIMENSION_BUFFER;

	default:
		assert(false);
		return D3D12_RESOURCE_DIMENSION_UNKNOWN;
	}
}


DXGI_FORMAT GetBaseFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_TYPELESS;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8A8_TYPELESS;

	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8X8_TYPELESS;

		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_R32G8X24_TYPELESS;

		// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_R32_TYPELESS;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_R24G8_TYPELESS;

		// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
		return DXGI_FORMAT_R16_TYPELESS;

	default:
		return format;
	}
}


DXGI_FORMAT GetUAVFormat(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
		return DXGI_FORMAT_R8G8B8A8_UNORM;

	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8A8_UNORM;

	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return DXGI_FORMAT_B8G8R8X8_UNORM;

	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

#ifdef _DEBUG
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_D16_UNORM:

		assert_msg(false, "Requested a UAV format for a depth stencil format.");
#endif

	default:
		return format;
	}
}


DXGI_FORMAT GetDSVFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;

		// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_D32_FLOAT;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_D24_UNORM_S8_UINT;

		// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
		return DXGI_FORMAT_D16_UNORM;

	default:
		return format;
	}
}


DXGI_FORMAT GetDepthFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;

		// No Stencil
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
		return DXGI_FORMAT_R32_FLOAT;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;

		// 16-bit Z w/o Stencil
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
		return DXGI_FORMAT_R16_UNORM;

	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}


DXGI_FORMAT GetStencilFormat(DXGI_FORMAT format)
{
	switch (format)
	{
		// 32-bit Z w/ Stencil
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return DXGI_FORMAT_X32_TYPELESS_G8X24_UINT;

		// 24-bit Z
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
		return DXGI_FORMAT_X24_TYPELESS_G8_UINT;

	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}


size_t BytesPerPixel(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R32G32B32A32_TYPELESS:
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
	case DXGI_FORMAT_R32G32B32A32_UINT:
	case DXGI_FORMAT_R32G32B32A32_SINT:
		return 16;

	case DXGI_FORMAT_R32G32B32_TYPELESS:
	case DXGI_FORMAT_R32G32B32_FLOAT:
	case DXGI_FORMAT_R32G32B32_UINT:
	case DXGI_FORMAT_R32G32B32_SINT:
		return 12;

	case DXGI_FORMAT_R16G16B16A16_TYPELESS:
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
	case DXGI_FORMAT_R16G16B16A16_UNORM:
	case DXGI_FORMAT_R16G16B16A16_UINT:
	case DXGI_FORMAT_R16G16B16A16_SNORM:
	case DXGI_FORMAT_R16G16B16A16_SINT:
	case DXGI_FORMAT_R32G32_TYPELESS:
	case DXGI_FORMAT_R32G32_FLOAT:
	case DXGI_FORMAT_R32G32_UINT:
	case DXGI_FORMAT_R32G32_SINT:
	case DXGI_FORMAT_R32G8X24_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
	case DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
	case DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
		return 8;

	case DXGI_FORMAT_R10G10B10A2_TYPELESS:
	case DXGI_FORMAT_R10G10B10A2_UNORM:
	case DXGI_FORMAT_R10G10B10A2_UINT:
	case DXGI_FORMAT_R11G11B10_FLOAT:
	case DXGI_FORMAT_R8G8B8A8_TYPELESS:
	case DXGI_FORMAT_R8G8B8A8_UNORM:
	case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
	case DXGI_FORMAT_R8G8B8A8_UINT:
	case DXGI_FORMAT_R8G8B8A8_SNORM:
	case DXGI_FORMAT_R8G8B8A8_SINT:
	case DXGI_FORMAT_R16G16_TYPELESS:
	case DXGI_FORMAT_R16G16_FLOAT:
	case DXGI_FORMAT_R16G16_UNORM:
	case DXGI_FORMAT_R16G16_UINT:
	case DXGI_FORMAT_R16G16_SNORM:
	case DXGI_FORMAT_R16G16_SINT:
	case DXGI_FORMAT_R32_TYPELESS:
	case DXGI_FORMAT_D32_FLOAT:
	case DXGI_FORMAT_R32_FLOAT:
	case DXGI_FORMAT_R32_UINT:
	case DXGI_FORMAT_R32_SINT:
	case DXGI_FORMAT_R24G8_TYPELESS:
	case DXGI_FORMAT_D24_UNORM_S8_UINT:
	case DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
	case DXGI_FORMAT_X24_TYPELESS_G8_UINT:
	case DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_B8G8R8A8_UNORM:
	case DXGI_FORMAT_B8G8R8X8_UNORM:
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
	case DXGI_FORMAT_B8G8R8A8_TYPELESS:
	case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
	case DXGI_FORMAT_B8G8R8X8_TYPELESS:
	case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
		return 4;

	case DXGI_FORMAT_R8G8_TYPELESS:
	case DXGI_FORMAT_R8G8_UNORM:
	case DXGI_FORMAT_R8G8_UINT:
	case DXGI_FORMAT_R8G8_SNORM:
	case DXGI_FORMAT_R8G8_SINT:
	case DXGI_FORMAT_R16_TYPELESS:
	case DXGI_FORMAT_R16_FLOAT:
	case DXGI_FORMAT_D16_UNORM:
	case DXGI_FORMAT_R16_UNORM:
	case DXGI_FORMAT_R16_UINT:
	case DXGI_FORMAT_R16_SNORM:
	case DXGI_FORMAT_R16_SINT:
	case DXGI_FORMAT_B5G6R5_UNORM:
	case DXGI_FORMAT_B5G5R5A1_UNORM:
	case DXGI_FORMAT_A8P8:
	case DXGI_FORMAT_B4G4R4A4_UNORM:
		return 2;

	case DXGI_FORMAT_R8_TYPELESS:
	case DXGI_FORMAT_R8_UNORM:
	case DXGI_FORMAT_R8_UINT:
	case DXGI_FORMAT_R8_SNORM:
	case DXGI_FORMAT_R8_SINT:
	case DXGI_FORMAT_A8_UNORM:
	case DXGI_FORMAT_P8:
		return 1;

	default:
		return 0;
	}
}


D3D12_RESOURCE_DESC DescribeTex2D(uint32_t width, uint32_t height, uint32_t arraySize,
	uint32_t numMips, uint32_t numSamples, Format format, uint32_t flags)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = (UINT16)arraySize;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	desc.Flags = (D3D12_RESOURCE_FLAGS)flags;
	desc.Format = GetBaseFormat(static_cast<DXGI_FORMAT>(format));
	desc.Height = (UINT)height;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.MipLevels = (UINT16)numMips;
	desc.SampleDesc.Count = numSamples;
	desc.SampleDesc.Quality = 0;
	desc.Width = (UINT64)width;

	return desc;
}


D3D12_RESOURCE_DESC DescribeTex3D(uint32_t width, uint32_t height, uint32_t depth,
	uint32_t numMips, uint32_t numSamples, Format format, uint32_t flags)
{
	D3D12_RESOURCE_DESC desc = {};
	desc.Alignment = 0;
	desc.DepthOrArraySize = (UINT16)depth;
	desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
	desc.Flags = (D3D12_RESOURCE_FLAGS)flags;
	desc.Format = GetBaseFormat(static_cast<DXGI_FORMAT>(format));
	desc.Height = (UINT)height;
	desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	desc.MipLevels = (UINT16)numMips;
	desc.SampleDesc.Count = numSamples;
	desc.SampleDesc.Quality = 0;
	desc.Width = (UINT64)width;

	return desc;
}


void CreateTextureResource(const string& name, const D3D12_RESOURCE_DESC& resourceDesc, D3D12_CLEAR_VALUE clearValue, ID3D12Resource** ppResource)
{
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_DEFAULT);
	assert_succeeded(GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
		&resourceDesc, D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_PPV_ARGS(ppResource)));

	SetDebugName(*ppResource, name);
}


D3D12_RESOURCE_STATES GetResourceState(ResourceState state)
{
	switch (state)
	{
	case ResourceState::Common:
		return D3D12_RESOURCE_STATE_COMMON;
	case ResourceState::VertexBuffer:
	case ResourceState::ConstantBuffer:
		return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
	case ResourceState::IndexBuffer:
		return D3D12_RESOURCE_STATE_INDEX_BUFFER;
	case ResourceState::RenderTarget:
		return D3D12_RESOURCE_STATE_RENDER_TARGET;
	case ResourceState::UnorderedAccess:
		return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	case ResourceState::DepthWrite:
		return D3D12_RESOURCE_STATE_DEPTH_WRITE;
	case ResourceState::DepthRead:
		return D3D12_RESOURCE_STATE_DEPTH_READ;
	case ResourceState::NonPixelShaderResource:
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
	case ResourceState::PixelShaderResource:
		return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case ResourceState::ShaderResource:
		return D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	case ResourceState::StreamOut:
		return D3D12_RESOURCE_STATE_STREAM_OUT;
	case ResourceState::IndirectArgument:
		return D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT;
	case ResourceState::CopyDest:
		return D3D12_RESOURCE_STATE_COPY_DEST;
	case ResourceState::CopySource:
		return D3D12_RESOURCE_STATE_COPY_SOURCE;
	case ResourceState::ResolveDest:
		return D3D12_RESOURCE_STATE_RESOLVE_DEST;
	case ResourceState::ResolveSource:
		return D3D12_RESOURCE_STATE_RESOLVE_SOURCE;
	case ResourceState::GenericRead:
		return D3D12_RESOURCE_STATE_GENERIC_READ;
	case ResourceState::Present:
		return D3D12_RESOURCE_STATE_PRESENT;
	case ResourceState::Predication:
		return D3D12_RESOURCE_STATE_PREDICATION;

	default:
		assert(false);
		return D3D12_RESOURCE_STATE_COMMON;
	}
}


D3D12_QUERY_HEAP_TYPE GetQueryHeapType(QueryHeapType type)
{
	switch (type)
	{
	case QueryHeapType::Occlusion:
		return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
	case QueryHeapType::Timestamp:
		return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
	case QueryHeapType::PipelineStats:
		return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;

	default:
		assert(false);
		return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
	}
}


D3D12_QUERY_TYPE GetQueryType(QueryType type)
{
	switch (type)
	{
	case QueryType::Occlusion:
		return D3D12_QUERY_TYPE_OCCLUSION;
	case QueryType::Timestamp:
		return D3D12_QUERY_TYPE_TIMESTAMP;
	case QueryType::PipelineStats:
		return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;

	default:
		assert(false);
		return D3D12_QUERY_TYPE_OCCLUSION;
	}
}

} // namespace Kodiak