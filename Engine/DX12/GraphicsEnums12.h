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

enum class ResourceState
{
	Undefined =					-1,
	Common =					D3D12_RESOURCE_STATE_COMMON,
	VertexBuffer =				D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	IndexBuffer =				D3D12_RESOURCE_STATE_INDEX_BUFFER,
	ConstantBuffer =			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER,
	Clear =						D3D12_RESOURCE_STATE_RENDER_TARGET,
	RenderTarget =				D3D12_RESOURCE_STATE_RENDER_TARGET,
	UnorderedAccess =			D3D12_RESOURCE_STATE_UNORDERED_ACCESS,
	DepthWrite =				D3D12_RESOURCE_STATE_DEPTH_WRITE,
	DepthRead =					D3D12_RESOURCE_STATE_DEPTH_READ,
	NonPixelShaderResource =	D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE,
	PixelShaderResource =		D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
	StreamOut =					D3D12_RESOURCE_STATE_STREAM_OUT,
	IndirectArgument =			D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT,
	CopyDest =					D3D12_RESOURCE_STATE_COPY_DEST,
	CopySource =				D3D12_RESOURCE_STATE_COPY_SOURCE,
	ResolveDest =				D3D12_RESOURCE_STATE_RESOLVE_DEST,
	ResolveSource =				D3D12_RESOURCE_STATE_RESOLVE_SOURCE,
	GenericRead =				D3D12_RESOURCE_STATE_GENERIC_READ,
	Present =					D3D12_RESOURCE_STATE_PRESENT,
	Predication =				D3D12_RESOURCE_STATE_PREDICATION
};

template <> struct EnableBitmaskOperators<ResourceState> { static const bool enable = true; };


enum class Blend
{
	Zero =				D3D12_BLEND_ZERO,
	One =				D3D12_BLEND_ONE,
	SrcColor =			D3D12_BLEND_SRC_COLOR,
	InvSrcColor =		D3D12_BLEND_INV_SRC_COLOR,
	SrcAlpha =			D3D12_BLEND_SRC_ALPHA,
	InvSrcAlpha =		D3D12_BLEND_INV_SRC_ALPHA,
	DstAlpha =			D3D12_BLEND_DEST_ALPHA,
	InvDstAlpha =		D3D12_BLEND_INV_DEST_ALPHA,
	DstColor =			D3D12_BLEND_DEST_COLOR,
	InvDstColor =		D3D12_BLEND_INV_DEST_COLOR,
	SrcAlphaSat =		D3D12_BLEND_SRC_ALPHA_SAT,
	BlendFactor =		D3D12_BLEND_BLEND_FACTOR,
	InvBlendFactor =	D3D12_BLEND_INV_BLEND_FACTOR,
	Src1Color =			D3D12_BLEND_SRC1_COLOR,
	InvSrc1Color =		D3D12_BLEND_INV_SRC1_COLOR,
	Src1Alpha =			D3D12_BLEND_SRC1_ALPHA,
	InvSrc1Alpha =		D3D12_BLEND_INV_SRC1_ALPHA
};


enum class BlendOp
{
	Add =			D3D12_BLEND_OP_ADD,
	Subtract =		D3D12_BLEND_OP_SUBTRACT,
	RevSubtract =	D3D12_BLEND_OP_REV_SUBTRACT,
	Min =			D3D12_BLEND_OP_MIN,
	Max =			D3D12_BLEND_OP_MAX
};


enum class LogicOp
{
	Clear =				D3D12_LOGIC_OP_CLEAR,
	Set =				D3D12_LOGIC_OP_SET,
	Copy =				D3D12_LOGIC_OP_COPY,
	CopyInverted =		D3D12_LOGIC_OP_COPY_INVERTED,
	Noop =				D3D12_LOGIC_OP_NOOP,
	Invert =			D3D12_LOGIC_OP_INVERT,
	And =				D3D12_LOGIC_OP_AND,
	Nand =				D3D12_LOGIC_OP_NAND,
	Or =				D3D12_LOGIC_OP_OR,
	Nor =				D3D12_LOGIC_OP_NOR,
	Xor =				D3D12_LOGIC_OP_XOR,
	Equiv =				D3D12_LOGIC_OP_EQUIV,
	AndReverse =		D3D12_LOGIC_OP_AND_REVERSE,
	OrReverse =			D3D12_LOGIC_OP_OR_REVERSE,
	OrInverted =		D3D12_LOGIC_OP_OR_INVERTED
};


enum class ColorWrite
{
	None =		0,
	Red =		1,
	Green =		2,
	Blue =		4,
	Alpha =		8,
	All =		Red | Green | Blue | Alpha
};

template <> struct EnableBitmaskOperators<ColorWrite> { static const bool enable = true; };


enum class DepthWrite
{
	Zero =	D3D12_DEPTH_WRITE_MASK_ZERO,
	All =	D3D12_DEPTH_WRITE_MASK_ALL
};


enum class CullMode
{
	None =	D3D12_CULL_MODE_NONE,
	Front =	D3D12_CULL_MODE_FRONT,
	Back =	D3D12_CULL_MODE_BACK
};


enum class FillMode
{
	Wireframe = D3D12_FILL_MODE_WIREFRAME,
	Solid =		D3D12_FILL_MODE_SOLID
};


enum class ComparisonFunc
{
	Never =			D3D12_COMPARISON_FUNC_NEVER,
	Less =			D3D12_COMPARISON_FUNC_LESS,
	Equal =			D3D12_COMPARISON_FUNC_EQUAL,
	LessEqual =		D3D12_COMPARISON_FUNC_LESS_EQUAL,
	Greater =		D3D12_COMPARISON_FUNC_GREATER,
	NotEqual =		D3D12_COMPARISON_FUNC_NOT_EQUAL,
	GreaterEqual =	D3D12_COMPARISON_FUNC_GREATER_EQUAL,
	Always =		D3D12_COMPARISON_FUNC_ALWAYS
};


enum class StencilOp
{
	Keep =		D3D12_STENCIL_OP_KEEP,
	Zero =		D3D12_STENCIL_OP_ZERO,
	Replace =	D3D12_STENCIL_OP_REPLACE,
	IncrSat =	D3D12_STENCIL_OP_INCR_SAT,
	DecrSat =	D3D12_STENCIL_OP_DECR_SAT,
	Invert =	D3D12_STENCIL_OP_INVERT,
	Incr =		D3D12_STENCIL_OP_INCR,
	Decr =		D3D12_STENCIL_OP_DECR
};


enum class IndexBufferStripCutValue
{
	Disabled =			D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED,
	Value_0xFFFF =		D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFF,
	Value_0xFFFFFFFF =	D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_0xFFFFFFFF
};


enum class PrimitiveTopologyType
{
	Undefined = D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED,
	Point =		D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT,
	Line =		D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE,
	Triangle =	D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
	Patch =		D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH
};


enum class InputClassification
{
	PerVertexData = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
	PerInstanceData = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA
};


enum class Format
{
	Unknown =				DXGI_FORMAT_UNKNOWN,
	B4G4R4A4_UNorm =		DXGI_FORMAT_B4G4R4A4_UNORM,
	B5G6R5_UNorm =			DXGI_FORMAT_B5G6R5_UNORM,
	B5G5R5A1_UNorm =		DXGI_FORMAT_B5G5R5A1_UNORM,
	B8G8R8A8_UNorm =		DXGI_FORMAT_B8G8R8A8_UNORM,
	R8_UNorm =				DXGI_FORMAT_R8_UNORM,
	R8_SNorm =				DXGI_FORMAT_R8_SNORM,
	R8_UInt =				DXGI_FORMAT_R8_UINT,
	R8_SInt =				DXGI_FORMAT_R8_SINT,
	R8G8_UNorm =			DXGI_FORMAT_R8G8_UNORM,
	R8G8_SNorm =			DXGI_FORMAT_R8G8_SNORM,
	R8G8_UInt =				DXGI_FORMAT_R8G8_UINT,
	R8G8_SInt =				DXGI_FORMAT_R8G8_SINT,
	R8G8B8A8_UNorm =		DXGI_FORMAT_R8G8B8A8_UNORM,
	R8G8B8A8_UNorm_SRGB =	DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
	R8G8B8A8_SNorm =		DXGI_FORMAT_R8G8B8A8_SNORM,
	R8G8B8A8_UInt =			DXGI_FORMAT_R8G8B8A8_UINT,
	R8G8B8A8_SInt =			DXGI_FORMAT_R8G8B8A8_SINT,
	R9G9B9E5_Float =		DXGI_FORMAT_R9G9B9E5_SHAREDEXP,
	R16_UNorm =				DXGI_FORMAT_R16_UNORM,
	R16_SNorm =				DXGI_FORMAT_R16_SNORM,
	R16_UInt =				DXGI_FORMAT_R16_UINT,
	R16_SInt =				DXGI_FORMAT_R16_SINT,
	R16_Float =				DXGI_FORMAT_R16_FLOAT,
	R16G16_UNorm =			DXGI_FORMAT_R16G16_UNORM,
	R16G16_SNorm =			DXGI_FORMAT_R16G16_SNORM,
	R16G16_UInt =			DXGI_FORMAT_R16G16_UINT,
	R16G16_SInt =			DXGI_FORMAT_R16G16_SINT,
	R16G16_Float =			DXGI_FORMAT_R16G16_FLOAT,
	R16G16B16A16_UNorm =	DXGI_FORMAT_R16G16B16A16_UNORM,
	R16G16B16A16_SNorm =	DXGI_FORMAT_R16G16B16A16_SNORM,
	R16G16B16A16_UInt =		DXGI_FORMAT_R16G16B16A16_UINT,
	R16G16B16A16_SInt =		DXGI_FORMAT_R16G16B16A16_SINT,
	R16G16B16A16_Float =	DXGI_FORMAT_R16G16B16A16_FLOAT,
	R32_UInt =				DXGI_FORMAT_R32_UINT,
	R32_SInt =				DXGI_FORMAT_R32_SINT,
	R32_Float =				DXGI_FORMAT_R32_FLOAT,
	R32G32_UInt =			DXGI_FORMAT_R32G32_UINT,
	R32G32_SInt =			DXGI_FORMAT_R32G32_SINT,
	R32G32_Float =			DXGI_FORMAT_R32G32_FLOAT,
	R32G32B32_UInt =		DXGI_FORMAT_R32G32B32_UINT,
	R32G32B32_SInt =		DXGI_FORMAT_R32G32B32_SINT,
	R32G32B32_Float =		DXGI_FORMAT_R32G32B32_FLOAT,
	R32G32B32A32_UInt =		DXGI_FORMAT_R32G32B32A32_UINT,
	R32G32B32A32_SInt =		DXGI_FORMAT_R32G32B32A32_SINT,
	R32G32B32A32_Float =	DXGI_FORMAT_R32G32B32A32_FLOAT,
	R11G11B10_Float =		DXGI_FORMAT_R11G11B10_FLOAT,
	R10G10B10A2_UNorm =		DXGI_FORMAT_R10G10B10A2_UNORM,

	D16_UNorm =				DXGI_FORMAT_D16_UNORM,
	D24S8 =					DXGI_FORMAT_D24_UNORM_S8_UINT,
	D32_Float =				DXGI_FORMAT_D32_FLOAT,
	D32_Float_S8_UInt =		DXGI_FORMAT_D32_FLOAT_S8X24_UINT
};

Format MapDXGIFormatToEngine(DXGI_FORMAT format);

} // namespace Kodiak
