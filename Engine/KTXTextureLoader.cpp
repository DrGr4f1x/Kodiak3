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

#include "KTXTextureLoader.h"

#include "Texture.h"


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


enum class KTXInternalFormat
{
	RGB_UNORM = 0x1907,		//GL_RGB
	BGR_UNORM = 0x80E0,		//GL_BGR
	RGBA_UNORM = 0x1908,	//GL_RGBA
	BGRA_UNORM = 0x80E1,	//GL_BGRA
	BGRA8_UNORM = 0x93A1,	//GL_BGRA8_EXT

	// unorm formats
	R8_UNORM = 0x8229,		//GL_R8
	RG8_UNORM = 0x822B,		//GL_RG8
	RGB8_UNORM = 0x8051,	//GL_RGB8
	RGBA8_UNORM = 0x8058,	//GL_RGBA8

	R16_UNORM = 0x822A,		//GL_R16
	RG16_UNORM = 0x822C,	//GL_RG16
	RGB16_UNORM = 0x8054,	//GL_RGB16
	RGBA16_UNORM = 0x805B,	//GL_RGBA16

	RGB10A2_UNORM = 0x8059,	//GL_RGB10_A2
	RGB10A2_SNORM_EXT = 0xFFFC,

	// snorm formats
	R8_SNORM = 0x8F94,		//GL_R8_SNORM
	RG8_SNORM = 0x8F95,		//GL_RG8_SNORM
	RGB8_SNORM = 0x8F96,	//GL_RGB8_SNORM
	RGBA8_SNORM = 0x8F97,	//GL_RGBA8_SNORM

	R16_SNORM = 0x8F98,		//GL_R16_SNORM
	RG16_SNORM = 0x8F99,	//GL_RG16_SNORM
	RGB16_SNORM = 0x8F9A,	//GL_RGB16_SNORM
	RGBA16_SNORM = 0x8F9B,	//GL_RGBA16_SNORM

	// unsigned integer formats
	R8U = 0x8232,			//GL_R8UI
	RG8U = 0x8238,			//GL_RG8UI
	RGB8U = 0x8D7D,			//GL_RGB8UI
	RGBA8U = 0x8D7C,		//GL_RGBA8UI

	R16U = 0x8234,			//GL_R16UI
	RG16U = 0x823A,			//GL_RG16UI
	RGB16U = 0x8D77,		//GL_RGB16UI
	RGBA16U = 0x8D76,		//GL_RGBA16UI

	R32U = 0x8236,			//GL_R32UI
	RG32U = 0x823C,			//GL_RG32UI
	RGB32U = 0x8D71,		//GL_RGB32UI
	RGBA32U = 0x8D70,		//GL_RGBA32UI

	RGB10A2U = 0x906F,		//GL_RGB10_A2UI
	RGB10A2I_EXT = 0xFFFB,

	// signed integer formats
	R8I = 0x8231,			//GL_R8I
	RG8I = 0x8237,			//GL_RG8I
	RGB8I = 0x8D8F,			//GL_RGB8I
	RGBA8I = 0x8D8E,		//GL_RGBA8I

	R16I = 0x8233,			//GL_R16I
	RG16I = 0x8239,			//GL_RG16I
	RGB16I = 0x8D89,		//GL_RGB16I
	RGBA16I = 0x8D88,		//GL_RGBA16I

	R32I = 0x8235,			//GL_R32I
	RG32I = 0x823B,			//GL_RG32I
	RGB32I = 0x8D83,		//GL_RGB32I
	RGBA32I = 0x8D82,		//GL_RGBA32I

	// Floating formats
	R16F = 0x822D,			//GL_R16F
	RG16F = 0x822F,			//GL_RG16F
	RGB16F = 0x881B,		//GL_RGB16F
	RGBA16F = 0x881A,		//GL_RGBA16F

	R32F = 0x822E,			//GL_R32F
	RG32F = 0x8230,			//GL_RG32F
	RGB32F = 0x8815,		//GL_RGB32F
	RGBA32F = 0x8814,		//GL_RGBA32F

	R64F_EXT = 0xFFFA,		//GL_R64F
	RG64F_EXT = 0xFFF9,		//GL_RG64F
	RGB64F_EXT = 0xFFF8,	//GL_RGB64F
	RGBA64F_EXT = 0xFFF7,	//GL_RGBA64F

	// sRGB formats
	SR8 = 0x8FBD,			//GL_SR8_EXT
	SRG8 = 0x8FBE,			//GL_SRG8_EXT
	SRGB8 = 0x8C41,			//GL_SRGB8
	SRGB8_ALPHA8 = 0x8C43,	//GL_SRGB8_ALPHA8

	// Packed formats
	RGB9E5 = 0x8C3D,		//GL_RGB9_E5
	RG11B10F = 0x8C3A,		//GL_R11F_G11F_B10F
	RG3B2 = 0x2A10,			//GL_R3_G3_B2
	R5G6B5 = 0x8D62,		//GL_RGB565
	RGB5A1 = 0x8057,		//GL_RGB5_A1
	RGBA4 = 0x8056,			//GL_RGBA4

	RG4_EXT = 0xFFFE,

	// Luminance Alpha formats
	LA4 = 0x8043,			//GL_LUMINANCE4_ALPHA4
	L8 = 0x8040,			//GL_LUMINANCE8
	A8 = 0x803C,			//GL_ALPHA8
	LA8 = 0x8045,			//GL_LUMINANCE8_ALPHA8
	L16 = 0x8042,			//GL_LUMINANCE16
	A16 = 0x803E,			//GL_ALPHA16
	LA16 = 0x8048,			//GL_LUMINANCE16_ALPHA16

	// Depth formats
	D16 = 0x81A5,			//GL_DEPTH_COMPONENT16
	D24 = 0x81A6,			//GL_DEPTH_COMPONENT24
	D16S8_EXT = 0xFFF6,
	D24S8 = 0x88F0,			//GL_DEPTH24_STENCIL8
	D32 = 0x81A7,			//GL_DEPTH_COMPONENT32
	D32F = 0x8CAC,			//GL_DEPTH_COMPONENT32F
	D32FS8X24 = 0x8CAD,		//GL_DEPTH32F_STENCIL8
	S8_EXT = 0x8D48,		//GL_STENCIL_INDEX8

	// Compressed formats
	RGB_DXT1 = 0x83F0,					//GL_COMPRESSED_RGB_S3TC_DXT1_EXT
	RGBA_DXT1 = 0x83F1,					//GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	RGBA_DXT3 = 0x83F2,					//GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	RGBA_DXT5 = 0x83F3,					//GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	R_ATI1N_UNORM = 0x8DBB,				//GL_COMPRESSED_RED_RGTC1
	R_ATI1N_SNORM = 0x8DBC,				//GL_COMPRESSED_SIGNED_RED_RGTC1
	RG_ATI2N_UNORM = 0x8DBD,			//GL_COMPRESSED_RG_RGTC2
	RG_ATI2N_SNORM = 0x8DBE,			//GL_COMPRESSED_SIGNED_RG_RGTC2
	RGB_BP_UNSIGNED_FLOAT = 0x8E8F,		//GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT
	RGB_BP_SIGNED_FLOAT = 0x8E8E,		//GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT
	RGB_BP_UNORM = 0x8E8C,				//GL_COMPRESSED_RGBA_BPTC_UNORM
	RGB_PVRTC_4BPPV1 = 0x8C00,			//GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
	RGB_PVRTC_2BPPV1 = 0x8C01,			//GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
	RGBA_PVRTC_4BPPV1 = 0x8C02,			//GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
	RGBA_PVRTC_2BPPV1 = 0x8C03,			//GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
	RGBA_PVRTC_4BPPV2 = 0x9137,			//GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG
	RGBA_PVRTC_2BPPV2 = 0x9138,			//GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG
	ATC_RGB = 0x8C92,					//GL_ATC_RGB_AMD
	ATC_RGBA_EXPLICIT_ALPHA = 0x8C93,	//GL_ATC_RGBA_EXPLICIT_ALPHA_AMD
	ATC_RGBA_INTERPOLATED_ALPHA = 0x87EE,	//GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD

	RGB_ETC = 0x8D64,					//GL_COMPRESSED_RGB8_ETC1
	RGB_ETC2 = 0x9274,					//GL_COMPRESSED_RGB8_ETC2
	RGBA_PUNCHTHROUGH_ETC2 = 0x9276,	//GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
	RGBA_ETC2 = 0x9278,					//GL_COMPRESSED_RGBA8_ETC2_EAC
	R11_EAC = 0x9270,					//GL_COMPRESSED_R11_EAC
	SIGNED_R11_EAC = 0x9271,			//GL_COMPRESSED_SIGNED_R11_EAC
	RG11_EAC = 0x9272,					//GL_COMPRESSED_RG11_EAC
	SIGNED_RG11_EAC = 0x9273,			//GL_COMPRESSED_SIGNED_RG11_EAC

	RGBA_ASTC_4x4 = 0x93B0,				//GL_COMPRESSED_RGBA_ASTC_4x4_KHR
	RGBA_ASTC_5x4 = 0x93B1,				//GL_COMPRESSED_RGBA_ASTC_5x4_KHR
	RGBA_ASTC_5x5 = 0x93B2,				//GL_COMPRESSED_RGBA_ASTC_5x5_KHR
	RGBA_ASTC_6x5 = 0x93B3,				//GL_COMPRESSED_RGBA_ASTC_6x5_KHR
	RGBA_ASTC_6x6 = 0x93B4,				//GL_COMPRESSED_RGBA_ASTC_6x6_KHR
	RGBA_ASTC_8x5 = 0x93B5,				//GL_COMPRESSED_RGBA_ASTC_8x5_KHR
	RGBA_ASTC_8x6 = 0x93B6,				//GL_COMPRESSED_RGBA_ASTC_8x6_KHR
	RGBA_ASTC_8x8 = 0x93B7,				//GL_COMPRESSED_RGBA_ASTC_8x8_KHR
	RGBA_ASTC_10x5 = 0x93B8,			//GL_COMPRESSED_RGBA_ASTC_10x5_KHR
	RGBA_ASTC_10x6 = 0x93B9,			//GL_COMPRESSED_RGBA_ASTC_10x6_KHR
	RGBA_ASTC_10x8 = 0x93BA,			//GL_COMPRESSED_RGBA_ASTC_10x8_KHR
	RGBA_ASTC_10x10 = 0x93BB,			//GL_COMPRESSED_RGBA_ASTC_10x10_KHR
	RGBA_ASTC_12x10 = 0x93BC,			//GL_COMPRESSED_RGBA_ASTC_12x10_KHR
	RGBA_ASTC_12x12 = 0x93BD,			//GL_COMPRESSED_RGBA_ASTC_12x12_KHR

	// sRGB formats
	SRGB_DXT1 = 0x8C4C,					//GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
	SRGB_ALPHA_DXT1 = 0x8C4D,			//GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
	SRGB_ALPHA_DXT3 = 0x8C4E,			//GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
	SRGB_ALPHA_DXT5 = 0x8C4F,			//GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
	SRGB_BP_UNORM = 0x8E8D,				//GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
	SRGB_PVRTC_2BPPV1 = 0x8A54,			//GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT
	SRGB_PVRTC_4BPPV1 = 0x8A55,			//GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT
	SRGB_ALPHA_PVRTC_2BPPV1 = 0x8A56,	//GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT
	SRGB_ALPHA_PVRTC_4BPPV1 = 0x8A57,	//GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT
	SRGB_ALPHA_PVRTC_2BPPV2 = 0x93F0,	//COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG
	SRGB_ALPHA_PVRTC_4BPPV2 = 0x93F1,	//GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG
	SRGB8_ETC2 = 0x9275,				//GL_COMPRESSED_SRGB8_ETC2
	SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277,	//GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
	SRGB8_ALPHA8_ETC2_EAC = 0x9279,		//GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
	SRGB8_ALPHA8_ASTC_4x4 = 0x93D0,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
	SRGB8_ALPHA8_ASTC_5x4 = 0x93D1,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR
	SRGB8_ALPHA8_ASTC_5x5 = 0x93D2,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR
	SRGB8_ALPHA8_ASTC_6x5 = 0x93D3,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR
	SRGB8_ALPHA8_ASTC_6x6 = 0x93D4,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR
	SRGB8_ALPHA8_ASTC_8x5 = 0x93D5,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR
	SRGB8_ALPHA8_ASTC_8x6 = 0x93D6,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR
	SRGB8_ALPHA8_ASTC_8x8 = 0x93D7,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR
	SRGB8_ALPHA8_ASTC_10x5 = 0x93D8,	//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR
	SRGB8_ALPHA8_ASTC_10x6 = 0x93D9,	//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR
	SRGB8_ALPHA8_ASTC_10x8 = 0x93DA,	//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR
	SRGB8_ALPHA8_ASTC_10x10 = 0x93DB,	//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR
	SRGB8_ALPHA8_ASTC_12x10 = 0x93DC,	//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR
	SRGB8_ALPHA8_ASTC_12x12 = 0x93DD,	//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR

	ALPHA8 = 0x803C,
	ALPHA16 = 0x803E,
	LUMINANCE8 = 0x8040,
	LUMINANCE16 = 0x8042,
	LUMINANCE8_ALPHA8 = 0x8045,
	LUMINANCE16_ALPHA16 = 0x8048,

	R8_USCALED_GTC = 0xF000,
	R8_SSCALED_GTC,
	RG8_USCALED_GTC,
	RG8_SSCALED_GTC,
	RGB8_USCALED_GTC,
	RGB8_SSCALED_GTC,
	RGBA8_USCALED_GTC,
	RGBA8_SSCALED_GTC,
	RGB10A2_USCALED_GTC,
	RGB10A2_SSCALED_GTC,
	R16_USCALED_GTC,
	R16_SSCALED_GTC,
	RG16_USCALED_GTC,
	RG16_SSCALED_GTC,
	RGB16_USCALED_GTC,
	RGB16_SSCALED_GTC,
	RGBA16_USCALED_GTC,
	RGBA16_SSCALED_GTC,
};

enum class KTXExternalFormat
{
	NONE = 0,					//GL_NONE
	RED = 0x1903,				//GL_RED
	RG = 0x8227,				//GL_RG
	RGB = 0x1907,				//GL_RGB
	BGR = 0x80E0,				//GL_BGR
	RGBA = 0x1908,				//GL_RGBA
	BGRA = 0x80E1,				//GL_BGRA
	RED_INTEGER = 0x8D94,		//GL_RED_INTEGER
	RG_INTEGER = 0x8228,		//GL_RG_INTEGER
	RGB_INTEGER = 0x8D98,		//GL_RGB_INTEGER
	BGR_INTEGER = 0x8D9A,		//GL_BGR_INTEGER
	RGBA_INTEGER = 0x8D99,		//GL_RGBA_INTEGER
	BGRA_INTEGER = 0x8D9B,		//GL_BGRA_INTEGER
	DEPTH = 0x1902,				//GL_DEPTH_COMPONENT
	DEPTH_STENCIL = 0x84F9,		//GL_DEPTH_STENCIL
	STENCIL = 0x1901,			//GL_STENCIL_INDEX

	LUMINANCE = 0x1909,			//GL_LUMINANCE
	ALPHA = 0x1906,				//GL_ALPHA
	LUMINANCE_ALPHA = 0x190A,	//GL_LUMINANCE_ALPHA

	SRGB_EXT = 0x8C40,			//SRGB_EXT
	SRGB_ALPHA_EXT = 0x8C42		//SRGB_ALPHA_EXT
};


enum class KTXTypeFormat
{
	NONE = 0,						//GL_NONE
	I8 = 0x1400,					//GL_BYTE
	U8 = 0x1401,					//GL_UNSIGNED_BYTE
	I16 = 0x1402,					//GL_SHORT
	U16 = 0x1403,					//GL_UNSIGNED_SHORT
	I32 = 0x1404,					//GL_INT
	U32 = 0x1405,					//GL_UNSIGNED_INT
	I64 = 0x140E,					//GL_INT64_ARB
	U64 = 0x140F,					//GL_UNSIGNED_INT64_ARB
	F16 = 0x140B,					//GL_HALF_FLOAT
	F16_OES = 0x8D61,				//GL_HALF_FLOAT_OES
	F32 = 0x1406,					//GL_FLOAT
	F64 = 0x140A,					//GL_DOUBLE
	UINT32_RGB9_E5_REV = 0x8C3E,	//GL_UNSIGNED_INT_5_9_9_9_REV
	UINT32_RG11B10F_REV = 0x8C3B,	//GL_UNSIGNED_INT_10F_11F_11F_REV
	UINT8_RG3B2 = 0x8032,			//GL_UNSIGNED_BYTE_3_3_2
	UINT8_RG3B2_REV = 0x8362,		//GL_UNSIGNED_BYTE_2_3_3_REV
	UINT16_RGB5A1 = 0x8034,			//GL_UNSIGNED_SHORT_5_5_5_1
	UINT16_RGB5A1_REV = 0x8366,		//GL_UNSIGNED_SHORT_1_5_5_5_REV
	UINT16_R5G6B5 = 0x8363,			//GL_UNSIGNED_SHORT_5_6_5
	UINT16_R5G6B5_REV = 0x8364,		//GL_UNSIGNED_SHORT_5_6_5_REV
	UINT16_RGBA4 = 0x8033,			//GL_UNSIGNED_SHORT_4_4_4_4
	UINT16_RGBA4_REV = 0x8365,		//GL_UNSIGNED_SHORT_4_4_4_4_REV
	UINT32_RGBA8 = 0x8035,			//GL_UNSIGNED_SHORT_8_8_8_8
	UINT32_RGBA8_REV = 0x8367,		//GL_UNSIGNED_SHORT_8_8_8_8_REV
	UINT32_RGB10A2 = 0x8036,		//GL_UNSIGNED_INT_10_10_10_2
	UINT32_RGB10A2_REV = 0x8368,	//GL_UNSIGNED_INT_2_10_10_10_REV

	UINT8_RG4_REV_GTC = 0xFFFD,
	UINT16_A1RGB5_GTC = 0xFFFC
};


struct FormatDesc
{
	KTXInternalFormat	glInternalFormat;
	KTXExternalFormat	glFormat;
	KTXTypeFormat		glType;
	Format				format;
};


Format MapKTXFormatToEngine(uint32_t internalFormat, uint32_t format, uint32_t type)
{
	static const KTXExternalFormat s_externalBGR = KTXExternalFormat::BGR;
	static const KTXExternalFormat s_externalBGRA = KTXExternalFormat::BGRA;
	static const KTXExternalFormat s_externalBGRInt = KTXExternalFormat::BGR_INTEGER;
	static const KTXExternalFormat s_externalBGRAInt = KTXExternalFormat::BGRA_INTEGER;

	static const KTXExternalFormat s_externalSRGB8 = KTXExternalFormat::RGB;
	static const KTXExternalFormat s_externalSRGB8_A8 = KTXExternalFormat::RGBA;

	static const KTXInternalFormat s_internalBGRA = KTXInternalFormat::BGRA8_UNORM;
	static const KTXInternalFormat s_internalRGBETC = KTXInternalFormat::RGB_ETC;

	static const KTXInternalFormat s_internalLuminance8 = KTXInternalFormat::LUMINANCE8;
	static const KTXInternalFormat s_internalAlpha8 = KTXInternalFormat::ALPHA8;
	static const KTXInternalFormat s_internalLuminanceAlpha8 = KTXInternalFormat::LUMINANCE8_ALPHA8;

	static const KTXInternalFormat s_internalLuminance16 = KTXInternalFormat::LUMINANCE16;
	static const KTXInternalFormat s_internalAlpha16 = KTXInternalFormat::ALPHA16;
	static const KTXInternalFormat s_internalLuminanceAlpha16 = KTXInternalFormat::LUMINANCE16_ALPHA16;

	static const KTXExternalFormat s_externalLuminance = KTXExternalFormat::LUMINANCE;
	static const KTXExternalFormat s_externalAlpha = KTXExternalFormat::ALPHA;
	static const KTXExternalFormat s_externalLuminanceAlpha = KTXExternalFormat::LUMINANCE_ALPHA;

	static const FormatDesc table[] =
	{
		{ KTXInternalFormat::RG4_EXT, KTXExternalFormat::RG, KTXTypeFormat::UINT8_RG4_REV_GTC, Format::Unknown },		//FORMAT_R4G4_UNORM,
		{ KTXInternalFormat::RGBA4, KTXExternalFormat::RGBA, KTXTypeFormat::UINT16_RGBA4_REV, Format::Unknown },		//FORMAT_RGBA4_UNORM,
		{ KTXInternalFormat::RGBA4, KTXExternalFormat::RGBA, KTXTypeFormat::UINT16_RGBA4, Format::B4G4R4A4_UNorm },		//FORMAT_BGRA4_UNORM,
		{ KTXInternalFormat::R5G6B5, KTXExternalFormat::RGB, KTXTypeFormat::UINT16_R5G6B5_REV, Format::Unknown },		//FORMAT_R5G6B5_UNORM,
		{ KTXInternalFormat::R5G6B5, KTXExternalFormat::RGB, KTXTypeFormat::UINT16_R5G6B5, Format::B5G6R5_UNorm },		//FORMAT_B5G6R5_UNORM,
		{ KTXInternalFormat::RGB5A1, KTXExternalFormat::RGBA, KTXTypeFormat::UINT16_RGB5A1_REV, Format::Unknown },		//FORMAT_RGB5A1_UNORM,
		{ KTXInternalFormat::RGB5A1, KTXExternalFormat::RGBA, KTXTypeFormat::UINT16_RGB5A1, Format::B5G5R5A1_UNorm },	//FORMAT_BGR5A1_UNORM,
		{ KTXInternalFormat::RGB5A1, KTXExternalFormat::RGBA, KTXTypeFormat::UINT16_A1RGB5_GTC, Format::Unknown },		//FORMAT_A1RGB5_UNORM,

		{ KTXInternalFormat::R8_UNORM, KTXExternalFormat::RED, KTXTypeFormat::U8, Format::R8_UNorm },		//FORMAT_R8_UNORM,
		{ KTXInternalFormat::R8_SNORM, KTXExternalFormat::RED, KTXTypeFormat::I8, Format::R8_SNorm },		//FORMAT_R8_SNORM,
		{ KTXInternalFormat::R8_USCALED_GTC, KTXExternalFormat::RED, KTXTypeFormat::U8, Format::Unknown },	//FORMAT_R8_USCALED,
		{ KTXInternalFormat::R8_SSCALED_GTC, KTXExternalFormat::RED, KTXTypeFormat::I8, Format::Unknown },	//FORMAT_R8_SSCALED,
		{ KTXInternalFormat::R8U, KTXExternalFormat::RED_INTEGER, KTXTypeFormat::U8, Format::R8_UInt },		//FORMAT_R8_UINT,
		{ KTXInternalFormat::R8I, KTXExternalFormat::RED_INTEGER, KTXTypeFormat::I8, Format::R8_SInt },		//FORMAT_R8_SINT,
		{ KTXInternalFormat::SR8, KTXExternalFormat::RED, KTXTypeFormat::U8, Format::Unknown },				//FORMAT_R8_SRGB,

		{ KTXInternalFormat::RG8_UNORM, KTXExternalFormat::RG, KTXTypeFormat::U8, Format::R8G8_UNorm },		//FORMAT_RG8_UNORM,
		{ KTXInternalFormat::RG8_SNORM, KTXExternalFormat::RG, KTXTypeFormat::I8, Format::R8G8_SNorm },		//FORMAT_RG8_SNORM,
		{ KTXInternalFormat::RG8_USCALED_GTC, KTXExternalFormat::RG, KTXTypeFormat::U8, Format::Unknown },	//FORMAT_RG8_USCALED,
		{ KTXInternalFormat::RG8_SSCALED_GTC, KTXExternalFormat::RG, KTXTypeFormat::I8, Format::Unknown },	//FORMAT_RG8_SSCALED,
		{ KTXInternalFormat::RG8U, KTXExternalFormat::RG_INTEGER, KTXTypeFormat::U8, Format::R8G8_UInt },	//FORMAT_RG8_UINT,
		{ KTXInternalFormat::RG8I, KTXExternalFormat::RG_INTEGER, KTXTypeFormat::I8, Format::R8G8_SInt },	//FORMAT_RG8_SINT,
		{ KTXInternalFormat::SRG8, KTXExternalFormat::RG, KTXTypeFormat::U8, Format::Unknown },				//FORMAT_RG8_SRGB,

		{ KTXInternalFormat::RGB8_UNORM, KTXExternalFormat::RGB, KTXTypeFormat::U8, Format::Unknown },			//FORMAT_RGB8_UNORM,
		{ KTXInternalFormat::RGB8_SNORM, KTXExternalFormat::RGB, KTXTypeFormat::I8, Format::Unknown },			//FORMAT_RGB8_SNORM,
		{ KTXInternalFormat::RGB8_USCALED_GTC, KTXExternalFormat::RGB, KTXTypeFormat::U8, Format::Unknown },	//FORMAT_RGB8_USCALED,
		{ KTXInternalFormat::RGB8_SSCALED_GTC, KTXExternalFormat::RGB, KTXTypeFormat::I8, Format::Unknown },	//FORMAT_RGB8_SSCALED,
		{ KTXInternalFormat::RGB8U, KTXExternalFormat::RGB_INTEGER, KTXTypeFormat::U8, Format::Unknown },		//FORMAT_RGB8_UINT,
		{ KTXInternalFormat::RGB8I, KTXExternalFormat::RGB_INTEGER, KTXTypeFormat::I8, Format::Unknown },		//FORMAT_RGB8_SINT,
		{ KTXInternalFormat::SRGB8, s_externalSRGB8, KTXTypeFormat::U8, Format::Unknown },						//FORMAT_RGB8_SRGB,

		{ KTXInternalFormat::RGB8_UNORM, s_externalBGR, KTXTypeFormat::U8, Format::Unknown },			//FORMAT_BGR8_UNORM_PACK8,
		{ KTXInternalFormat::RGB8_SNORM, s_externalBGR, KTXTypeFormat::I8, Format::Unknown },			//FORMAT_BGR8_SNORM_PACK8,
		{ KTXInternalFormat::RGB8_USCALED_GTC, s_externalBGR, KTXTypeFormat::U8, Format::Unknown },		//FORMAT_BGR8_USCALED_PACK8,
		{ KTXInternalFormat::RGB8_SSCALED_GTC, s_externalBGR, KTXTypeFormat::I8, Format::Unknown },		//FORMAT_BGR8_SSCALED_PACK8,
		{ KTXInternalFormat::RGB8U, s_externalBGRInt, KTXTypeFormat::U8, Format::Unknown },				//FORMAT_BGR8_UINT_PACK8,
		{ KTXInternalFormat::RGB8I, s_externalBGRInt, KTXTypeFormat::I8, Format::Unknown },				//FORMAT_BGR8_SINT_PACK8,
		{ KTXInternalFormat::SRGB8, s_externalBGR, KTXTypeFormat::U8, Format::Unknown },				//FORMAT_BGR8_SRGB_PACK8,

		{ KTXInternalFormat::RGBA8_UNORM, KTXExternalFormat::RGBA, KTXTypeFormat::U8, Format::R8G8B8A8_UNorm },		//FORMAT_RGBA8_UNORM_PACK8,
		{ KTXInternalFormat::RGBA8_SNORM, KTXExternalFormat::RGBA, KTXTypeFormat::I8, Format::R8G8B8A8_SNorm },		//FORMAT_RGBA8_SNORM_PACK8,
		{ KTXInternalFormat::RGBA8_USCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::U8, Format::Unknown },		//FORMAT_RGBA8_USCALED_PACK8,
		{ KTXInternalFormat::RGBA8_SSCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::I8, Format::Unknown },		//FORMAT_RGBA8_SSCALED_PACK8,
		{ KTXInternalFormat::RGBA8U, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::U8, Format::R8G8B8A8_UInt },	//FORMAT_RGBA8_UINT_PACK8,
		{ KTXInternalFormat::RGBA8I, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::I8, Format::R8G8B8A8_SInt },	//FORMAT_RGBA8_SINT_PACK8,
		{ KTXInternalFormat::SRGB8_ALPHA8, s_externalSRGB8_A8, KTXTypeFormat::U8, Format::R8G8B8A8_UNorm_SRGB },	//FORMAT_RGBA8_SRGB_PACK8,

		{ s_internalBGRA, s_externalBGRA, KTXTypeFormat::U8, Format::Unknown },							//FORMAT_BGRA8_UNORM_PACK8,
		{ KTXInternalFormat::RGBA8_SNORM, s_externalBGRA, KTXTypeFormat::I8, Format::Unknown },			//FORMAT_BGRA8_SNORM_PACK8,
		{ KTXInternalFormat::RGBA8_USCALED_GTC, s_externalBGRA, KTXTypeFormat::U8, Format::Unknown },	//FORMAT_BGRA8_USCALED_PACK8,
		{ KTXInternalFormat::RGBA8_SSCALED_GTC, s_externalBGRA, KTXTypeFormat::I8, Format::Unknown },	//FORMAT_BGRA8_SSCALED_PACK8,
		{ KTXInternalFormat::RGBA8U, s_externalBGRAInt, KTXTypeFormat::U8, Format::Unknown },			//FORMAT_BGRA8_UINT_PACK8,
		{ KTXInternalFormat::RGBA8I, s_externalBGRAInt, KTXTypeFormat::I8, Format::Unknown },			//FORMAT_BGRA8_SINT_PACK8,
		{ KTXInternalFormat::SRGB8_ALPHA8, s_externalBGRA, KTXTypeFormat::U8, Format::Unknown },		//FORMAT_BGRA8_SRGB_PACK8,

		{ KTXInternalFormat::RGBA8_UNORM, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGBA8_REV, Format::Unknown },			//FORMAT_ABGR8_UNORM_PACK32,
		{ KTXInternalFormat::RGBA8_SNORM, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGBA8_REV, Format::Unknown },			//FORMAT_ABGR8_SNORM_PACK32,
		{ KTXInternalFormat::RGBA8_USCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGBA8_REV, Format::Unknown },	//FORMAT_ABGR8_USCALED_PACK32,
		{ KTXInternalFormat::RGBA8_SSCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGBA8_REV, Format::Unknown },	//FORMAT_ABGR8_SSCALED_PACK32,
		{ KTXInternalFormat::RGBA8U, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::UINT32_RGBA8_REV, Format::Unknown },		//FORMAT_ABGR8_UINT_PACK32,
		{ KTXInternalFormat::RGBA8I, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::UINT32_RGBA8_REV, Format::Unknown },		//FORMAT_ABGR8_SINT_PACK32,
		{ KTXInternalFormat::SRGB8_ALPHA8, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGBA8_REV, Format::Unknown },			//FORMAT_ABGR8_SRGB_PACK32,

		{ KTXInternalFormat::RGB10A2_UNORM, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },			//FORMAT_RGB10A2_UNORM_PACK32,
		{ KTXInternalFormat::RGB10A2_SNORM_EXT, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },		//FORMAT_RGB10A2_SNORM_PACK32,
		{ KTXInternalFormat::RGB10A2_USCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },	//FORMAT_RGB10A2_USCALE_PACK32,
		{ KTXInternalFormat::RGB10A2_SSCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },	//FORMAT_RGB10A2_SSCALE_PACK32,
		{ KTXInternalFormat::RGB10A2U, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },		//FORMAT_RGB10A2_UINT_PACK32,
		{ KTXInternalFormat::RGB10A2I_EXT, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },	//FORMAT_RGB10A2_SINT_PACK32,

		{ KTXInternalFormat::RGB10A2_UNORM, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGB10A2, Format::R10G10B10A2_UNorm },	//FORMAT_BGR10A2_UNORM_PACK32,
		{ KTXInternalFormat::RGB10A2_SNORM_EXT, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGB10A2, Format::Unknown },			//FORMAT_BGR10A2_SNORM_PACK32,
		{ KTXInternalFormat::RGB10A2_USCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGB10A2, Format::Unknown },		//FORMAT_BGR10A2_USCALE_PACK32,
		{ KTXInternalFormat::RGB10A2_SSCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::UINT32_RGB10A2, Format::Unknown },		//FORMAT_BGR10A2_SSCALE_PACK32,
		{ KTXInternalFormat::RGB10A2U, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::UINT32_RGB10A2, Format::Unknown },			//FORMAT_BGR10A2_UINT_PACK32,
		{ KTXInternalFormat::RGB10A2I_EXT, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::UINT32_RGB10A2, Format::Unknown },		//FORMAT_BGR10A2_SINT_PACK32,

		{ KTXInternalFormat::R16_UNORM, KTXExternalFormat::RED, KTXTypeFormat::U16, Format::R16_UNorm },		//FORMAT_R16_UNORM_PACK16,
		{ KTXInternalFormat::R16_SNORM, KTXExternalFormat::RED, KTXTypeFormat::I16, Format::R16_SNorm },		//FORMAT_R16_SNORM_PACK16,
		{ KTXInternalFormat::R16_USCALED_GTC, KTXExternalFormat::RED, KTXTypeFormat::U16, Format::Unknown },	//FORMAT_R16_USCALED_PACK16,
		{ KTXInternalFormat::R16_SSCALED_GTC, KTXExternalFormat::RED, KTXTypeFormat::I16, Format::Unknown },	//FORMAT_R16_SSCALED_PACK16,
		{ KTXInternalFormat::R16U, KTXExternalFormat::RED_INTEGER, KTXTypeFormat::U16, Format::R16_UInt },		//FORMAT_R16_UINT_PACK16,
		{ KTXInternalFormat::R16I, KTXExternalFormat::RED_INTEGER, KTXTypeFormat::I16, Format::R16_SInt },		//FORMAT_R16_SINT_PACK16,
		{ KTXInternalFormat::R16F, KTXExternalFormat::RED, KTXTypeFormat::F16, Format::R16_Float },				//FORMAT_R16_SFLOAT_PACK16,

		{ KTXInternalFormat::RG16_UNORM, KTXExternalFormat::RG, KTXTypeFormat::U16, Format::R16G16_UNorm },		//FORMAT_RG16_UNORM_PACK16,
		{ KTXInternalFormat::RG16_SNORM, KTXExternalFormat::RG, KTXTypeFormat::I16, Format::R16G16_SNorm },		//FORMAT_RG16_SNORM_PACK16,
		{ KTXInternalFormat::RG16_USCALED_GTC, KTXExternalFormat::RG, KTXTypeFormat::U16, Format::Unknown },	//FORMAT_RG16_USCALED_PACK16,
		{ KTXInternalFormat::RG16_SSCALED_GTC, KTXExternalFormat::RG, KTXTypeFormat::I16, Format::Unknown },	//FORMAT_RG16_SSCALED_PACK16,
		{ KTXInternalFormat::RG16U, KTXExternalFormat::RG_INTEGER, KTXTypeFormat::U16, Format::R16G16_UInt },	//FORMAT_RG16_UINT_PACK16,
		{ KTXInternalFormat::RG16I, KTXExternalFormat::RG_INTEGER, KTXTypeFormat::I16, Format::R16G16_SInt },	//FORMAT_RG16_SINT_PACK16,
		{ KTXInternalFormat::RG16F, KTXExternalFormat::RG, KTXTypeFormat::F16, Format::R16G16_Float },			//FORMAT_RG16_SFLOAT_PACK16,

		{ KTXInternalFormat::RGB16_UNORM, KTXExternalFormat::RGB, KTXTypeFormat::U16, Format::Unknown },		//FORMAT_RGB16_UNORM_PACK16,
		{ KTXInternalFormat::RGB16_SNORM, KTXExternalFormat::RGB, KTXTypeFormat::I16, Format::Unknown },		//FORMAT_RGB16_SNORM_PACK16,
		{ KTXInternalFormat::RGB16_USCALED_GTC, KTXExternalFormat::RGB, KTXTypeFormat::U16, Format::Unknown },	//FORMAT_RGB16_USCALED_PACK16,
		{ KTXInternalFormat::RGB16_SSCALED_GTC, KTXExternalFormat::RGB, KTXTypeFormat::I16, Format::Unknown },	//FORMAT_RGB16_USCALED_PACK16,
		{ KTXInternalFormat::RGB16U, KTXExternalFormat::RGB_INTEGER, KTXTypeFormat::U16, Format::Unknown },		//FORMAT_RGB16_UINT_PACK16,
		{ KTXInternalFormat::RGB16I, KTXExternalFormat::RGB_INTEGER, KTXTypeFormat::I16, Format::Unknown },		//FORMAT_RGB16_SINT_PACK16,
		{ KTXInternalFormat::RGB16F, KTXExternalFormat::RGB, KTXTypeFormat::F16, Format::Unknown },				//FORMAT_RGB16_SFLOAT_PACK16,

		{ KTXInternalFormat::RGBA16_UNORM, KTXExternalFormat::RGBA, KTXTypeFormat::U16, Format::R16G16B16A16_UNorm },	//FORMAT_RGBA16_UNORM_PACK16,
		{ KTXInternalFormat::RGBA16_SNORM, KTXExternalFormat::RGBA, KTXTypeFormat::I16, Format::R16G16B16A16_SNorm },	//FORMAT_RGBA16_SNORM_PACK16,
		{ KTXInternalFormat::RGBA16_USCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::U16, Format::Unknown },		//FORMAT_RGBA16_USCALED_PACK16,
		{ KTXInternalFormat::RGBA16_SSCALED_GTC, KTXExternalFormat::RGBA, KTXTypeFormat::I16, Format::Unknown },		//FORMAT_RGBA16_SSCALED_PACK16,
		{ KTXInternalFormat::RGBA16U, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::U16, Format::R16G16B16A16_UInt },	//FORMAT_RGBA16_UINT_PACK16,
		{ KTXInternalFormat::RGBA16I, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::I16, Format::R16G16B16A16_SInt },	//FORMAT_RGBA16_SINT_PACK16,
		{ KTXInternalFormat::RGBA16F, KTXExternalFormat::RGBA, KTXTypeFormat::F16, Format::R16G16B16A16_Float },		//FORMAT_RGBA16_SFLOAT_PACK16,

		{ KTXInternalFormat::R32U, KTXExternalFormat::RED_INTEGER, KTXTypeFormat::U32, Format::R32_UInt },				//FORMAT_R32_UINT_PACK32,
		{ KTXInternalFormat::R32I, KTXExternalFormat::RED_INTEGER, KTXTypeFormat::I32, Format::R32_SInt },				//FORMAT_R32_SINT_PACK32,
		{ KTXInternalFormat::R32F, KTXExternalFormat::RED, KTXTypeFormat::F32, Format::R32_Float },						//FORMAT_R32_SFLOAT_PACK32,

		{ KTXInternalFormat::RG32U, KTXExternalFormat::RG_INTEGER, KTXTypeFormat::U32, Format::R32G32_UInt },			//FORMAT_RG32_UINT_PACK32,
		{ KTXInternalFormat::RG32I, KTXExternalFormat::RG_INTEGER, KTXTypeFormat::I32, Format::R32G32_SInt },			//FORMAT_RG32_SINT_PACK32,
		{ KTXInternalFormat::RG32F, KTXExternalFormat::RG, KTXTypeFormat::F32, Format::R32G32_Float },					//FORMAT_RG32_SFLOAT_PACK32,

		{ KTXInternalFormat::RGB32U, KTXExternalFormat::RGB_INTEGER, KTXTypeFormat::U32, Format::R32G32B32_UInt },		//FORMAT_RGB32_UINT_PACK32,
		{ KTXInternalFormat::RGB32I, KTXExternalFormat::RGB_INTEGER, KTXTypeFormat::I32, Format::R32G32B32_SInt },		//FORMAT_RGB32_SINT_PACK32,
		{ KTXInternalFormat::RGB32F, KTXExternalFormat::RGB, KTXTypeFormat::F32, Format::R32G32B32_Float },				//FORMAT_RGB32_SFLOAT_PACK32,

		{ KTXInternalFormat::RGBA32U, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::U32, Format::R32G32B32A32_UInt },	//FORMAT_RGBA32_UINT_PACK32,
		{ KTXInternalFormat::RGBA32I, KTXExternalFormat::RGBA_INTEGER, KTXTypeFormat::I32, Format::R32G32B32A32_SInt },	//FORMAT_RGBA32_SINT_PACK32,
		{ KTXInternalFormat::RGBA32F, KTXExternalFormat::RGBA, KTXTypeFormat::F32, Format::R32G32B32A32_Float },		//FORMAT_RGBA32_SFLOAT_PACK32,

		{ KTXInternalFormat::R64F_EXT, KTXExternalFormat::RED, KTXTypeFormat::U64, Format::Unknown },			//FORMAT_R64_UINT_PACK64,
		{ KTXInternalFormat::R64F_EXT, KTXExternalFormat::RED, KTXTypeFormat::I64, Format::Unknown },			//FORMAT_R64_SINT_PACK64,
		{ KTXInternalFormat::R64F_EXT, KTXExternalFormat::RED, KTXTypeFormat::F64, Format::Unknown },			//FORMAT_R64_SFLOAT_PACK64,

		{ KTXInternalFormat::RG64F_EXT, KTXExternalFormat::RG, KTXTypeFormat::U64, Format::Unknown },			//FORMAT_RG64_UINT_PACK64,
		{ KTXInternalFormat::RG64F_EXT, KTXExternalFormat::RG, KTXTypeFormat::I64, Format::Unknown },			//FORMAT_RG64_SINT_PACK64,
		{ KTXInternalFormat::RG64F_EXT, KTXExternalFormat::RG, KTXTypeFormat::F64, Format::Unknown },			//FORMAT_RG64_SFLOAT_PACK64,

		{ KTXInternalFormat::RGB64F_EXT, KTXExternalFormat::RGB, KTXTypeFormat::U64, Format::Unknown },		//FORMAT_RGB64_UINT_PACK64,
		{ KTXInternalFormat::RGB64F_EXT, KTXExternalFormat::RGB, KTXTypeFormat::I64, Format::Unknown },		//FORMAT_RGB64_SINT_PACK64,
		{ KTXInternalFormat::RGB64F_EXT, KTXExternalFormat::RGB, KTXTypeFormat::F64, Format::Unknown },		//FORMAT_RGB64_SFLOAT_PACK64,

		{ KTXInternalFormat::RGBA64F_EXT, KTXExternalFormat::RGBA, KTXTypeFormat::U64, Format::Unknown },		//FORMAT_RGBA64_UINT_PACK64,
		{ KTXInternalFormat::RGBA64F_EXT, KTXExternalFormat::RGBA, KTXTypeFormat::I64, Format::Unknown },		//FORMAT_RGBA64_SINT_PACK64,
		{ KTXInternalFormat::RGBA64F_EXT, KTXExternalFormat::RGBA, KTXTypeFormat::F64, Format::Unknown },		//FORMAT_RGBA64_SFLOAT_PACK64,

		{ KTXInternalFormat::RG11B10F, KTXExternalFormat::RGB, KTXTypeFormat::UINT32_RG11B10F_REV, Format::R11G11B10_Float },	//FORMAT_RG11B10_UFLOAT_PACK32,
		{ KTXInternalFormat::RGB9E5, KTXExternalFormat::RGB, KTXTypeFormat::UINT32_RGB9_E5_REV, Format::R9G9B9E5_Float },		//FORMAT_RGB9E5_UFLOAT_PACK32,

		{ KTXInternalFormat::D16, KTXExternalFormat::DEPTH, KTXTypeFormat::NONE, Format::D16_UNorm },							//FORMAT_D16_UNORM_PACK16,
		{ KTXInternalFormat::D24, KTXExternalFormat::DEPTH, KTXTypeFormat::NONE, Format::Unknown },							//FORMAT_D24_UNORM,
		{ KTXInternalFormat::D32F, KTXExternalFormat::DEPTH, KTXTypeFormat::NONE, Format::D32_Float },							//FORMAT_D32_UFLOAT,
		{ KTXInternalFormat::S8_EXT, KTXExternalFormat::STENCIL, KTXTypeFormat::NONE, Format::R8_UNorm },						//FORMAT_S8_UNORM,
		{ KTXInternalFormat::D16S8_EXT, KTXExternalFormat::DEPTH, KTXTypeFormat::NONE, Format::Unknown },						//FORMAT_D16_UNORM_S8_UINT_PACK32,
		{ KTXInternalFormat::D24S8, KTXExternalFormat::DEPTH_STENCIL, KTXTypeFormat::NONE, Format::D24S8 },					//FORMAT_D24_UNORM_S8_UINT_PACK32,
		{ KTXInternalFormat::D32FS8X24, KTXExternalFormat::DEPTH_STENCIL, KTXTypeFormat::NONE, Format::D32_Float_S8_UInt },	//FORMAT_D32_SFLOAT_S8_UINT_PACK64,

		{ KTXInternalFormat::RGB_DXT1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGB_DXT1_UNORM_BLOCK8,
		{ KTXInternalFormat::SRGB_DXT1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_DXT1_SRGB_BLOCK8,
		{ KTXInternalFormat::RGBA_DXT1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::BC1_UNorm },				//FORMAT_RGBA_DXT1_UNORM_BLOCK8,
		{ KTXInternalFormat::SRGB_ALPHA_DXT1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::BC1_UNorm_SRGB },	//FORMAT_RGBA_DXT1_SRGB_BLOCK8,
		{ KTXInternalFormat::RGBA_DXT3, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::BC2_UNorm },				//FORMAT_RGBA_DXT3_UNORM_BLOCK16,
		{ KTXInternalFormat::SRGB_ALPHA_DXT3, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::BC2_UNorm_SRGB },	//FORMAT_RGBA_DXT3_SRGB_BLOCK16,
		{ KTXInternalFormat::RGBA_DXT5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::BC3_UNorm },				//FORMAT_RGBA_DXT5_UNORM_BLOCK16,
		{ KTXInternalFormat::SRGB_ALPHA_DXT5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::BC3_UNorm_SRGB },	//FORMAT_RGBA_DXT5_SRGB_BLOCK16,
		{ KTXInternalFormat::R_ATI1N_UNORM, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_R_ATI1N_UNORM_BLOCK8,
		{ KTXInternalFormat::R_ATI1N_SNORM, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_R_ATI1N_SNORM_BLOCK8,
		{ KTXInternalFormat::RG_ATI2N_UNORM, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RG_ATI2N_UNORM_BLOCK16,
		{ KTXInternalFormat::RG_ATI2N_SNORM, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RG_ATI2N_SNORM_BLOCK16,
		{ KTXInternalFormat::RGB_BP_UNSIGNED_FLOAT, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },	//FORMAT_RGB_BP_UFLOAT_BLOCK16,
		{ KTXInternalFormat::RGB_BP_SIGNED_FLOAT, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },		//FORMAT_RGB_BP_SFLOAT_BLOCK16,
		{ KTXInternalFormat::RGB_BP_UNORM, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_BP_UNORM,
		{ KTXInternalFormat::SRGB_BP_UNORM, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGB_BP_SRGB,

		{ s_internalRGBETC, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },									//FORMAT_RGB_ETC2_UNORM_BLOCK8,
		{ KTXInternalFormat::SRGB8_ETC2, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },						//FORMAT_RGB_ETC2_SRGB_BLOCK8,
		{ KTXInternalFormat::RGBA_PUNCHTHROUGH_ETC2, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ETC2_PUNCHTHROUGH_UNORM,
		{ KTXInternalFormat::SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },	//FORMAT_RGBA_ETC2_PUNCHTHROUGH_SRGB,
		{ KTXInternalFormat::RGBA_ETC2, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },						//FORMAT_RGBA_ETC2_UNORM_BLOCK16,
		{ KTXInternalFormat::SRGB8_ALPHA8_ETC2_EAC, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ETC2_SRGB_BLOCK16,
		{ KTXInternalFormat::R11_EAC, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },							//FORMAT_R11_EAC_UNORM,
		{ KTXInternalFormat::SIGNED_R11_EAC, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_R11_EAC_SNORM,
		{ KTXInternalFormat::RG11_EAC, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },						//FORMAT_RG11_EAC_UNORM,
		{ KTXInternalFormat::SIGNED_RG11_EAC, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RG11_EAC_SNORM,

		{ KTXInternalFormat::RGBA_ASTC_4x4, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC4X4_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_4x4, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC4X4_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_5x4, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC5X4_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_5x4, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC5X4_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_5x5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC5X5_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_5x5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC5X5_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_6x5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC6X5_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_6x5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC6X5_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_6x6, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC6X6_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_6x6, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC6X6_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_8x5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC8X5_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_8x5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC8X5_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_8x6, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC8X6_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_8x6, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC8X6_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_8x8, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC8X8_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_8x8, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC8X8_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_10x5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC10X5_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_10x5, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC10X5_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_10x6, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC10X6_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_10x6, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC10X6_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_10x8, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC10X8_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_10x8, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC10X8_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_10x10, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC10X10_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_10x10, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC10X10_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_12x10, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC12X10_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_12x10, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC12X10_SRGB,
		{ KTXInternalFormat::RGBA_ASTC_12x12, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC12X12_UNORM,
		{ KTXInternalFormat::SRGB8_ALPHA8_ASTC_12x12, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC12X12_SRGB,

		{ KTXInternalFormat::RGB_PVRTC_4BPPV1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_PVRTC1_8X8_UNORM_BLOCK32,
		{ KTXInternalFormat::SRGB_PVRTC_2BPPV1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_PVRTC1_8X8_SRGB_BLOCK32,
		{ KTXInternalFormat::RGB_PVRTC_2BPPV1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_PVRTC1_16X8_UNORM_BLOCK32,
		{ KTXInternalFormat::SRGB_PVRTC_4BPPV1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_PVRTC1_16X8_SRGB_BLOCK32,
		{ KTXInternalFormat::RGBA_PVRTC_4BPPV1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_PVRTC1_8X8_UNORM_BLOCK32,
		{ KTXInternalFormat::SRGB_ALPHA_PVRTC_2BPPV1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_PVRTC1_8X8_SRGB_BLOCK32,
		{ KTXInternalFormat::RGBA_PVRTC_2BPPV1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_PVRTC1_16X8_UNORM_BLOCK32,
		{ KTXInternalFormat::SRGB_ALPHA_PVRTC_4BPPV1, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_PVRTC1_16X8_SRGB_BLOCK32,
		{ KTXInternalFormat::RGBA_PVRTC_4BPPV2, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_PVRTC2_4X4_UNORM_BLOCK8,
		{ KTXInternalFormat::SRGB_ALPHA_PVRTC_4BPPV2, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_PVRTC2_4X4_SRGB_BLOCK8,
		{ KTXInternalFormat::RGBA_PVRTC_2BPPV2, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_PVRTC2_8X4_UNORM_BLOCK8,
		{ KTXInternalFormat::SRGB_ALPHA_PVRTC_2BPPV2, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_PVRTC2_8X4_SRGB_BLOCK8,

		{ KTXInternalFormat::RGB_ETC, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },							//FORMAT_RGB_ETC_UNORM_BLOCK8,
		{ KTXInternalFormat::ATC_RGB, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },							//FORMAT_RGB_ATC_UNORM_BLOCK8,
		{ KTXInternalFormat::ATC_RGBA_EXPLICIT_ALPHA, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ATCA_UNORM_BLOCK16,
		{ KTXInternalFormat::ATC_RGBA_INTERPOLATED_ALPHA, KTXExternalFormat::NONE, KTXTypeFormat::NONE, Format::Unknown },		//FORMAT_RGBA_ATCI_UNORM_BLOCK16,

		{ s_internalLuminance8, s_externalLuminance, KTXTypeFormat::U8, Format::Unknown },						//FORMAT_L8_UNORM_PACK8,
		{ s_internalAlpha8, s_externalAlpha, KTXTypeFormat::U8, Format::Unknown },								//FORMAT_A8_UNORM_PACK8,
		{ s_internalLuminanceAlpha8, s_externalLuminanceAlpha, KTXTypeFormat::U8, Format::Unknown },				//FORMAT_LA8_UNORM_PACK8,
		{ s_internalLuminance16, s_externalLuminance, KTXTypeFormat::U16, Format::Unknown },						//FORMAT_L16_UNORM_PACK16,
		{ s_internalAlpha16, s_externalAlpha, KTXTypeFormat::U16, Format::Unknown },								//FORMAT_A16_UNORM_PACK16,
		{ s_internalLuminanceAlpha16, s_externalLuminanceAlpha, KTXTypeFormat::U16, Format::Unknown },			//FORMAT_LA16_UNORM_PACK16,

		{ KTXInternalFormat::RGB8_UNORM, s_externalBGRA, KTXTypeFormat::U8, Format::Unknown },					//FORMAT_BGRX8_UNORM,
		{ KTXInternalFormat::SRGB8, s_externalBGRA, KTXTypeFormat::U8, Format::Unknown },							//FORMAT_BGRX8_SRGB,

		{ KTXInternalFormat::RG3B2, KTXExternalFormat::RGB, KTXTypeFormat::UINT8_RG3B2_REV, Format::Unknown },		//FORMAT_RG3B2_UNORM,
	};

	for (uint32_t i = 0; i < _countof(table); ++i)
	{
		if (internalFormat != static_cast<uint32_t>(table[i].glInternalFormat))
			continue;
		if (format != static_cast<uint32_t>(table[i].glFormat))
			continue;
		if (type != static_cast<uint32_t>(table[i].glType))
			continue;
		return table[i].format;
	}

	return Format::Unknown;
}


inline TextureType GetTarget(const KTXHeader10& header)
{
	if (header.numberOfFaces > 1)
	{
		if (header.numberOfArrayElements > 0)
		{
			return TextureType::TextureCube_Array;
		}
		else
		{
			return TextureType::TextureCube;
		}
	}
	else if (header.numberOfArrayElements > 0)
	{
		if (header.pixelHeight == 0)
		{
			return TextureType::Texture1D_Array;
		}
		else
		{
			return TextureType::Texture2D_Array;
		}
	}
	else if (header.pixelHeight == 0)
	{
		return TextureType::Texture1D;
	}
	else if (header.pixelDepth > 0)
	{
		return TextureType::Texture3D;
	}
	else
	{
		return TextureType::Texture2D;
	}
}

} // anonymous namespace


HRESULT Kodiak::CreateKTXTextureFromMemory(
	const byte* ktxData,
	size_t ktxDataSize,
	size_t maxsize,
	bool forceSRGB,
	Texture* texture
)
{
	assert(ktxData && (ktxDataSize >= sizeof(KTXHeader10)));

	// Check the KTX magic number
	if (memcmp(ktxData, FOURCC_KTX10, sizeof(FOURCC_KTX10)) != 0)
	{
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	ktxData += sizeof(FOURCC_KTX10);
	ktxDataSize -= sizeof(FOURCC_KTX10);

	// Read header
	const KTXHeader10& header = *reinterpret_cast<const KTXHeader10 *>(ktxData);

	// Sanitize some values
	uint32_t arraySize = max(header.numberOfArrayElements, 1u);
	uint32_t numMips = max(header.numberOfMipmapLevels, 1u);
	uint32_t numFaces = max(header.numberOfFaces, 1u);
	uint32_t width = max(header.pixelWidth, 1u);
	uint32_t height = max(header.pixelHeight, 1u);
	uint32_t depth = max(header.pixelDepth, 1u);
	size_t offset = sizeof(KTXHeader10);

	// Skip key value data
	offset += header.bytesOfKeyValueData;

	auto format = MapKTXFormatToEngine(header.glInternalFormat, header.glFormat, header.glType);
	assert(format != Format::Unknown);

	if (forceSRGB)
	{
		format = MakeSRGB(format);
	}

	auto target = GetTarget(header);
	bool isCubeMap = (target == TextureType::TextureCube || target == TextureType::TextureCube_Array);

	// Bound sizes (for security purposes we don't trust KTX file metadata larger than the D3D 11.x hardware requirements)
	if (numMips > Limits::MaxTextureMipLevels)
	{
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	switch (target)
	{
	case TextureType::Texture1D:
	case TextureType::Texture1D_Array:
		if ((header.numberOfArrayElements > Limits::MaxTexture1DArrayElements) ||
			(header.pixelWidth > Limits::MaxTextureDimension1D))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	case TextureType::Texture2D:
	case TextureType::Texture2D_Array:
		if ((arraySize > Limits::MaxTexture2DArrayElements) ||
			(header.pixelWidth > Limits::MaxTextureDimension2D) ||
			(header.pixelHeight > Limits::MaxTextureDimension2D))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	case TextureType::TextureCube:
	case TextureType::TextureCube_Array:
		// This is the right bound because we set arraySize to (NumCubes*6) above
		if ((arraySize > Limits::MaxTexture2DArrayElements) ||
			(header.pixelWidth > Limits::MaxTextureDimensionCube) ||
			(header.pixelHeight > Limits::MaxTextureDimensionCube))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	case TextureType::Texture3D:
		if ((arraySize > 1) ||
			(header.pixelWidth > Limits::MaxTextureDimension3D) ||
			(header.pixelHeight > Limits::MaxTextureDimension3D) ||
			(header.pixelDepth > Limits::MaxTextureDimension3D))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	default:
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	// Create the texture
	if (target == TextureType::Texture1D)
	{
		texture->Create1D(width, format, ktxData + offset + sizeof(uint32_t));
	}
	else if (target == TextureType::Texture2D)
	{
		texture->Create2D(width, height, format, ktxData + offset + sizeof(uint32_t));
	}
	else if (target == TextureType::Texture2D_Array)
	{
		texture->Create2DArray(width, height, arraySize, format, ktxData + offset + sizeof(uint32_t));
	}
	else if (target == TextureType::TextureCube)
	{
		texture->CreateCube(width, height, format, ktxData + offset + sizeof(uint32_t));
	}
	else
	{
		assert_msg(false, "Only 1D and 2D textures are supported (for now)");
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	return S_OK;
}


HRESULT Kodiak::CreateKTXTextureFromFile(
	const char* szFileName,
	size_t maxsize,
	bool forceSRGB,
	Texture* texture
)
{
	HRESULT res = S_OK;
	assert(false);
	return res;
}