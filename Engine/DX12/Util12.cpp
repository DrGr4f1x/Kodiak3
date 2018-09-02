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

	case ResourceType::GenericBuffer:
	case ResourceType::IndexBuffer:
	case ResourceType::VertexBuffer:
	case ResourceType::StructuredBuffer:
	case ResourceType::TypedBuffer:
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

	case ResourceType::GenericBuffer:
	case ResourceType::IndexBuffer:
	case ResourceType::VertexBuffer:
	case ResourceType::StructuredBuffer:
	case ResourceType::TypedBuffer:
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

	case ResourceType::GenericBuffer:
	case ResourceType::IndexBuffer:
	case ResourceType::VertexBuffer:
	case ResourceType::StructuredBuffer:
	case ResourceType::TypedBuffer:
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

} // namespace Kodiak