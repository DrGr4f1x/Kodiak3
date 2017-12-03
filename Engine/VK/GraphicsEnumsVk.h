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
	Undefined = -1,
	Common,
	VertexBuffer,
	IndexBuffer,
	ConstantBuffer,
	Clear,
	RenderTarget,
	UnorderedAccess,
	DepthWrite,
	DepthRead,
	NonPixelShaderResource,
	PixelShaderResource,
	StreamOut,
	IndirectArgument,
	CopyDest,
	CopySource,
	ResolveDest,
	ResolveSource,
	GenericRead,
	Present,
	Predication
};

template <> struct EnableBitmaskOperators<ResourceState> { static const bool enable = true; };


enum class Blend
{
	Zero =				VK_BLEND_FACTOR_ZERO,
	One =				VK_BLEND_FACTOR_ONE,
	SrcColor =			VK_BLEND_FACTOR_SRC_COLOR,
	InvSrcColor =		VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,
	SrcAlpha =			VK_BLEND_FACTOR_SRC_ALPHA,
	InvSrcAlpha =		VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
	DstAlpha =			VK_BLEND_FACTOR_DST_ALPHA,
	InvDstAlpha =		VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,
	DstColor =			VK_BLEND_FACTOR_DST_COLOR,
	InvDstColor =		VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,
	SrcAlphaSat =		VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,
	BlendFactor =		VK_BLEND_FACTOR_CONSTANT_COLOR,
	InvBlendFactor =	VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR,
	Src1Color =			VK_BLEND_FACTOR_SRC1_COLOR,
	InvSrc1Color =		VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR,
	Src1Alpha =			VK_BLEND_FACTOR_SRC1_ALPHA,
	InvSrc1Alpha =		VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA
};


enum class BlendOp
{
	Add =			VK_BLEND_OP_ADD,
	Subtract =		VK_BLEND_OP_SUBTRACT,
	RevSubtract =	VK_BLEND_OP_REVERSE_SUBTRACT,
	Min =			VK_BLEND_OP_MIN,
	Max =			VK_BLEND_OP_MAX
};


enum class LogicOp
{
	Clear =			VK_LOGIC_OP_CLEAR,
	Set =			VK_LOGIC_OP_SET,
	Copy =			VK_LOGIC_OP_COPY,
	CopyInverted =	VK_LOGIC_OP_COPY_INVERTED,
	Noop =			VK_LOGIC_OP_NO_OP,
	Invert =		VK_LOGIC_OP_INVERT,
	And =			VK_LOGIC_OP_AND,
	Nand =			VK_LOGIC_OP_NAND,
	Or =			VK_LOGIC_OP_OR,
	Nor =			VK_LOGIC_OP_NOR,
	Xor =			VK_LOGIC_OP_XOR,
	Equiv =			VK_LOGIC_OP_EQUIVALENT,
	AndReverse =	VK_LOGIC_OP_AND_REVERSE,
	OrReverse =		VK_LOGIC_OP_OR_REVERSE,
	OrInverted =	VK_LOGIC_OP_OR_INVERTED
};


enum class ColorWrite
{
	None =		0,
	Red =		VK_COLOR_COMPONENT_R_BIT,
	Green =		VK_COLOR_COMPONENT_G_BIT,
	Blue =		VK_COLOR_COMPONENT_B_BIT,
	Alpha =		VK_COLOR_COMPONENT_A_BIT,
	All =		Red | Green | Blue | Alpha
};

template <> struct EnableBitmaskOperators<ColorWrite> { static const bool enable = true; };


enum class DepthWrite
{
	Zero =	0,
	All =	1
};


enum class CullMode
{
	None =	VK_CULL_MODE_NONE,
	Front = VK_CULL_MODE_FRONT_BIT,
	Back =	VK_CULL_MODE_BACK_BIT
};


enum class FillMode
{
	Wireframe = VK_POLYGON_MODE_LINE,
	Solid =		VK_POLYGON_MODE_FILL
};


enum class ComparisonFunc
{
	Never =			VK_COMPARE_OP_NEVER,
	Less =			VK_COMPARE_OP_LESS,
	Equal =			VK_COMPARE_OP_EQUAL,
	LessEqual =		VK_COMPARE_OP_LESS_OR_EQUAL,
	Greater =		VK_COMPARE_OP_GREATER,
	NotEqual =		VK_COMPARE_OP_NOT_EQUAL,
	GreaterEqual =	VK_COMPARE_OP_GREATER_OR_EQUAL,
	Always =		VK_COMPARE_OP_ALWAYS
};


enum class StencilOp
{
	Keep =		VK_STENCIL_OP_KEEP,
	Zero =		VK_STENCIL_OP_ZERO,
	Replace =	VK_STENCIL_OP_REPLACE,
	IncrSat =	VK_STENCIL_OP_INCREMENT_AND_CLAMP,
	DecrSat =	VK_STENCIL_OP_DECREMENT_AND_CLAMP,
	Invert =	VK_STENCIL_OP_INVERT,
	Incr =		VK_STENCIL_OP_INCREMENT_AND_WRAP,
	Decr =		VK_STENCIL_OP_DECREMENT_AND_WRAP
};


enum class IndexBufferStripCutValue
{
	Disabled,
	Value_0xFFFF,
	Value_0xFFFFFFFF
};


enum class PrimitiveTopology
{
	PointList =						VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
	LineList =						VK_PRIMITIVE_TOPOLOGY_LINE_LIST,
	LineStrip =						VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
	TriangleList =					VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	TriangleStrip =					VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP,
	LineListWithAdjacency =			VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY,
	LineStripWithAdjacency =		VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY,
	TriangleListWithAdjacency =		VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY,
	TriangleStripWithAdjacency =	VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY,
	Patch_1_ControlPoint =			VK_PRIMITIVE_TOPOLOGY_PATCH_LIST,
	Patch_2_ControlPoint,
	Patch_3_ControlPoint,
	Patch_4_ControlPoint,
	Patch_5_ControlPoint,
	Patch_6_ControlPoint,
	Patch_7_ControlPoint,
	Patch_8_ControlPoint,
	Patch_9_ControlPoint,
	Patch_10_ControlPoint,
	Patch_11_ControlPoint,
	Patch_12_ControlPoint,
	Patch_13_ControlPoint,
	Patch_14_ControlPoint,
	Patch_15_ControlPoint,
	Patch_16_ControlPoint,
	Patch_17_ControlPoint,
	Patch_18_ControlPoint,
	Patch_19_ControlPoint,
	Patch_20_ControlPoint,
	Patch_21_ControlPoint,
	Patch_22_ControlPoint,
	Patch_23_ControlPoint,
	Patch_24_ControlPoint,
	Patch_25_ControlPoint,
	Patch_26_ControlPoint,
	Patch_27_ControlPoint,
	Patch_28_ControlPoint,
	Patch_29_ControlPoint,
	Patch_30_ControlPoint,
	Patch_31_ControlPoint,
	Patch_32_ControlPoint
};

VkPrimitiveTopology MapEnginePrimitiveTopologyToVulkan(PrimitiveTopology topology);
uint32_t GetControlPointCount(PrimitiveTopology topology);


enum class InputClassification
{
	PerVertexData = VK_VERTEX_INPUT_RATE_VERTEX,
	PerInstanceData = VK_VERTEX_INPUT_RATE_INSTANCE
};


enum class ShaderVisibility
{
	All =		VK_SHADER_STAGE_ALL,
	Graphics =	VK_SHADER_STAGE_ALL_GRAPHICS,
	Compute =	VK_SHADER_STAGE_COMPUTE_BIT,
	Domain =	VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
	Geometry =	VK_SHADER_STAGE_GEOMETRY_BIT,
	Hull =		VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
	Pixel =		VK_SHADER_STAGE_FRAGMENT_BIT,
	Vertex =	VK_SHADER_STAGE_VERTEX_BIT
};


enum class DescriptorType
{
	CBV,
	Sampler,
	TextureSRV,
	ImageSRV,
	BufferUAV,
	ImageUAV
};

inline VkDescriptorType DescriptorTypeToVulkan(DescriptorType type)
{
	switch (type)
	{
	case DescriptorType::CBV:
		return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		break;
	case DescriptorType::Sampler:
		return VK_DESCRIPTOR_TYPE_SAMPLER;
		break;
	case DescriptorType::TextureSRV:
		return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
		break;
	case DescriptorType::ImageSRV:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		break;
	case DescriptorType::BufferUAV:
		return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		break;
	case DescriptorType::ImageUAV:
		return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
		break;
	}
	// Should never reach this assert
	assert(false);
	return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
}


enum class RootParameterType
{
	Invalid,
	Constants32Bit,
	DescriptorTable,
	CBV,
	UAV,
	ImageSRV,
	TextureSRV
};


enum class RootSignatureFlags
{
	None,
	AllowInputAssemblerInputLayout,
	DenyVertexShaderRootAccess,
	DenyHullShaderRootAccess,
	DenyDomainShaderRootAccess,
	DenyGeometryShaderRootAccess,
	DenyPixelShaderRootAccess,
	AllowStreamOutput
};

template <> struct EnableBitmaskOperators<RootSignatureFlags> { static const bool enable = true; };


enum class TextureFilter
{
	MinMagMipPoint,
	MinMagPointMipLinear,
	MinPointMagLinearMipPoint,
	MinPointMagMipLinear,
	MinLinearMagMipPoint,
	MinLinearMagPointMipLinear,
	MinMagLinearMipPoint,
	MinMagMipLinear,
	Anisotropic,

	ComparisonMinMagMipPoint,
	ComparisonMinMagPointMipLinear,
	ComparisonMinPointMagLinearMipPoint,
	ComparisonMinPointMagMipLinear,
	ComparisonMinLinearMagMipPoint,
	ComparisonMinLinearMagPointMipLinear,
	ComparisonMinMagLinearMipPoint,
	ComparisonMinMagMipLinear,
	ComparisonAnisotropic,

	MinimumMinMagMipPoint,
	MinimumMinMagPointMipLinear,
	MinimumMinPointMagLinearMipPoint,
	MinimumMinPointMagMipLinear,
	MinimumMinLinearMagMipPoint,
	MinimumMinLinearMagPointMipLinear,
	MinimumMinMagLinearMipPoint,
	MinimumMinMagMipLinear,
	MinimumAnisotropic,

	MaximumMinMagMipPoint,
	MaximumMinMagPointMipLinear,
	MaximumMinPointMagLinearMipPoint,
	MaximumMinPointMagMipLinear,
	MaximumMinLinearMagMipPoint,
	MaximumMinLinearMagPointMipLinear,
	MaximumMinMagLinearMipPoint,
	MaximumMinMagMipLinear,
	MaximumAnisotropic
};


enum class TextureAddress
{
	Wrap,
	Mirror,
	Clamp,
	Border,
	MirrorOnce
};


enum class Format
{
	Unknown =				VK_FORMAT_UNDEFINED,
	B4G4R4A4_UNorm =		VK_FORMAT_B4G4R4A4_UNORM_PACK16,
	B5G6R5_UNorm =			VK_FORMAT_B5G6R5_UNORM_PACK16,
	B5G5R5A1_UNorm =		VK_FORMAT_B5G5R5A1_UNORM_PACK16,
	B8G8R8A8_UNorm =		VK_FORMAT_B8G8R8A8_UNORM,
	R8_UNorm =				VK_FORMAT_R8_UNORM,
	R8_SNorm =				VK_FORMAT_R8_SNORM,
	R8_UInt =				VK_FORMAT_R8_UINT,
	R8_SInt =				VK_FORMAT_R8_SINT,
	R8G8_UNorm =			VK_FORMAT_R8G8_UNORM,
	R8G8_SNorm =			VK_FORMAT_R8G8_SNORM,
	R8G8_UInt =				VK_FORMAT_R8G8_UINT,
	R8G8_SInt =				VK_FORMAT_R8G8_SINT,
	R8G8B8A8_UNorm =		VK_FORMAT_R8G8B8A8_UNORM,
	R8G8B8A8_UNorm_SRGB =	VK_FORMAT_R8G8B8A8_SRGB,
	R8G8B8A8_SNorm =		VK_FORMAT_R8G8B8A8_SNORM,
	R8G8B8A8_UInt =			VK_FORMAT_R8G8B8A8_UINT,
	R8G8B8A8_SInt =			VK_FORMAT_R8G8B8A8_SINT,
	R9G9B9E5_Float =		VK_FORMAT_E5B9G9R9_UFLOAT_PACK32,
	R16_UNorm =				VK_FORMAT_R16_UNORM,
	R16_SNorm =				VK_FORMAT_R16_SNORM,
	R16_UInt =				VK_FORMAT_R16_UINT,
	R16_SInt =				VK_FORMAT_R16_SINT,
	R16_Float =				VK_FORMAT_R16_SFLOAT,
	R16G16_UNorm =			VK_FORMAT_R16G16_UNORM,
	R16G16_SNorm =			VK_FORMAT_R16G16_SNORM,
	R16G16_UInt =			VK_FORMAT_R16G16_UINT,
	R16G16_SInt =			VK_FORMAT_R16G16_SINT,
	R16G16_Float =			VK_FORMAT_R16G16_SFLOAT,
	R16G16B16A16_UNorm =	VK_FORMAT_R16G16B16A16_UNORM,
	R16G16B16A16_SNorm =	VK_FORMAT_R16G16B16A16_SNORM,
	R16G16B16A16_UInt =		VK_FORMAT_R16G16B16A16_UINT,
	R16G16B16A16_SInt =		VK_FORMAT_R16G16B16A16_SINT,
	R16G16B16A16_Float =	VK_FORMAT_R16G16B16A16_SFLOAT,
	R32_UInt =				VK_FORMAT_R32_UINT,
	R32_SInt =				VK_FORMAT_R32_SINT,
	R32_Float =				VK_FORMAT_R32_SFLOAT,
	R32G32_UInt =			VK_FORMAT_R32G32_UINT,
	R32G32_SInt =			VK_FORMAT_R32G32_SINT,
	R32G32_Float =			VK_FORMAT_R32G32_SFLOAT,
	R32G32B32_UInt =		VK_FORMAT_R32G32B32_UINT,
	R32G32B32_SInt =		VK_FORMAT_R32G32B32_SINT,
	R32G32B32_Float =		VK_FORMAT_R32G32B32_SFLOAT,
	R32G32B32A32_UInt =		VK_FORMAT_R32G32B32A32_UINT,
	R32G32B32A32_SInt =		VK_FORMAT_R32G32B32A32_SINT,
	R32G32B32A32_Float =	VK_FORMAT_R32G32B32A32_SFLOAT,
	R11G11B10_Float =		VK_FORMAT_B10G11R11_UFLOAT_PACK32,
	R10G10B10A2_UNorm =		VK_FORMAT_A2R10G10B10_UNORM_PACK32,

	D16_UNorm =				VK_FORMAT_D16_UNORM,
	D24S8 =					VK_FORMAT_D24_UNORM_S8_UINT,
	D32_Float =				VK_FORMAT_D32_SFLOAT,
	D32_Float_S8_UInt =		VK_FORMAT_D32_SFLOAT_S8_UINT,

	BC1_UNorm =				VK_FORMAT_BC1_RGBA_UNORM_BLOCK,
	BC1_UNorm_SRGB =		VK_FORMAT_BC1_RGBA_SRGB_BLOCK,
	BC2_UNorm =				VK_FORMAT_BC2_UNORM_BLOCK,
	BC2_UNorm_SRGB =		VK_FORMAT_BC2_SRGB_BLOCK,
	BC3_UNorm =				VK_FORMAT_BC3_UNORM_BLOCK,
	BC3_UNorm_SRGB =		VK_FORMAT_BC3_SRGB_BLOCK,
	BC4_UNorm =				VK_FORMAT_BC4_UNORM_BLOCK,
	BC4_SNorm =				VK_FORMAT_BC4_SNORM_BLOCK,
	BC5_UNorm =				VK_FORMAT_BC5_UNORM_BLOCK,
	BC5_SNorm =				VK_FORMAT_BC5_SNORM_BLOCK,
	BC6H_Float =			VK_FORMAT_BC6H_SFLOAT_BLOCK,
	BC6H_UFloat =			VK_FORMAT_BC6H_UFLOAT_BLOCK,
	BC7_UNorm =				VK_FORMAT_BC7_UNORM_BLOCK,
	BC7_UNorm_SRGB =		VK_FORMAT_BC7_SRGB_BLOCK
};

Format MapVulkanFormatToEngine(VkFormat format);


enum class CommandListType
{
	Direct,
	Bundle,
	Compute,
	Copy
};

} // namespace Kodiak