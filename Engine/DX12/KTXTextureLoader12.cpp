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

#include "CommandContext12.h"

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

enum class GLInternalFormat
{
	RGB_UNORM = 0x1907,		//GL_RGB
	BGR_UNORM = 0x80E0,		//GL_BGR
	RGBA_UNORM = 0x1908,		//GL_RGBA
	BGRA_UNORM = 0x80E1,		//GL_BGRA
	BGRA8_UNORM = 0x93A1,		//GL_BGRA8_EXT

	// unorm formats
	R8_UNORM = 0x8229,			//GL_R8
	RG8_UNORM = 0x822B,		//GL_RG8
	RGB8_UNORM = 0x8051,		//GL_RGB8
	RGBA8_UNORM = 0x8058,		//GL_RGBA8

	R16_UNORM = 0x822A,		//GL_R16
	RG16_UNORM = 0x822C,		//GL_RG16
	RGB16_UNORM = 0x8054,		//GL_RGB16
	RGBA16_UNORM = 0x805B,		//GL_RGBA16

	RGB10A2_UNORM = 0x8059,	//GL_RGB10_A2
	RGB10A2_SNORM_EXT = 0xFFFC,

	// snorm formats
	R8_SNORM = 0x8F94,			//GL_R8_SNORM
	RG8_SNORM = 0x8F95,		//GL_RG8_SNORM
	RGB8_SNORM = 0x8F96,		//GL_RGB8_SNORM
	RGBA8_SNORM = 0x8F97,		//GL_RGBA8_SNORM

	R16_SNORM = 0x8F98,		//GL_R16_SNORM
	RG16_SNORM = 0x8F99,		//GL_RG16_SNORM
	RGB16_SNORM = 0x8F9A,		//GL_RGB16_SNORM
	RGBA16_SNORM = 0x8F9B,		//GL_RGBA16_SNORM

	// unsigned integer formats
	R8U = 0x8232,				//GL_R8UI
	RG8U = 0x8238,				//GL_RG8UI
	RGB8U = 0x8D7D,			//GL_RGB8UI
	RGBA8U = 0x8D7C,			//GL_RGBA8UI

	R16U = 0x8234,				//GL_R16UI
	RG16U = 0x823A,			//GL_RG16UI
	RGB16U = 0x8D77,			//GL_RGB16UI
	RGBA16U = 0x8D76,			//GL_RGBA16UI

	R32U = 0x8236,				//GL_R32UI
	RG32U = 0x823C,			//GL_RG32UI
	RGB32U = 0x8D71,			//GL_RGB32UI
	RGBA32U = 0x8D70,			//GL_RGBA32UI

	RGB10A2U = 0x906F,			//GL_RGB10_A2UI
	RGB10A2I_EXT = 0xFFFB,

	// signed integer formats
	R8I = 0x8231,				//GL_R8I
	RG8I = 0x8237,				//GL_RG8I
	RGB8I = 0x8D8F,			//GL_RGB8I
	RGBA8I = 0x8D8E,			//GL_RGBA8I

	R16I = 0x8233,				//GL_R16I
	RG16I = 0x8239,			//GL_RG16I
	RGB16I = 0x8D89,			//GL_RGB16I
	RGBA16I = 0x8D88,			//GL_RGBA16I

	R32I = 0x8235,				//GL_R32I
	RG32I = 0x823B,			//GL_RG32I
	RGB32I = 0x8D83,			//GL_RGB32I
	RGBA32I = 0x8D82,			//GL_RGBA32I

	// Floating formats
	R16F = 0x822D,				//GL_R16F
	RG16F = 0x822F,			//GL_RG16F
	RGB16F = 0x881B,			//GL_RGB16F
	RGBA16F = 0x881A,			//GL_RGBA16F

	R32F = 0x822E,				//GL_R32F
	RG32F = 0x8230,			//GL_RG32F
	RGB32F = 0x8815,			//GL_RGB32F
	RGBA32F = 0x8814,			//GL_RGBA32F

	R64F_EXT = 0xFFFA,			//GL_R64F
	RG64F_EXT = 0xFFF9,		//GL_RG64F
	RGB64F_EXT = 0xFFF8,		//GL_RGB64F
	RGBA64F_EXT = 0xFFF7,		//GL_RGBA64F

	// sRGB formats
	SR8 = 0x8FBD,				//GL_SR8_EXT
	SRG8 = 0x8FBE,				//GL_SRG8_EXT
	SRGB8 = 0x8C41,			//GL_SRGB8
	SRGB8_ALPHA8 = 0x8C43,		//GL_SRGB8_ALPHA8

	// Packed formats
	RGB9E5 = 0x8C3D,			//GL_RGB9_E5
	RG11B10F = 0x8C3A,			//GL_R11F_G11F_B10F
	RG3B2 = 0x2A10,			//GL_R3_G3_B2
	R5G6B5 = 0x8D62,			//GL_RGB565
	RGB5A1 = 0x8057,			//GL_RGB5_A1
	RGBA4 = 0x8056,			//GL_RGBA4

	RG4_EXT = 0xFFFE,

	// Luminance Alpha formats
	LA4 = 0x8043,				//GL_LUMINANCE4_ALPHA4
	L8 = 0x8040,				//GL_LUMINANCE8
	A8 = 0x803C,				//GL_ALPHA8
	LA8 = 0x8045,				//GL_LUMINANCE8_ALPHA8
	L16 = 0x8042,				//GL_LUMINANCE16
	A16 = 0x803E,				//GL_ALPHA16
	LA16 = 0x8048,				//GL_LUMINANCE16_ALPHA16

	// Depth formats
	D16 = 0x81A5,				//GL_DEPTH_COMPONENT16
	D24 = 0x81A6,				//GL_DEPTH_COMPONENT24
	D16S8_EXT = 0xFFF6,
	D24S8 = 0x88F0,			//GL_DEPTH24_STENCIL8
	D32 = 0x81A7,				//GL_DEPTH_COMPONENT32
	D32F = 0x8CAC,				//GL_DEPTH_COMPONENT32F
	D32FS8X24 = 0x8CAD,		//GL_DEPTH32F_STENCIL8
	S8_EXT = 0x8D48,			//GL_STENCIL_INDEX8

	// Compressed formats
	RGB_DXT1 = 0x83F0,						//GL_COMPRESSED_RGB_S3TC_DXT1_EXT
	RGBA_DXT1 = 0x83F1,					//GL_COMPRESSED_RGBA_S3TC_DXT1_EXT
	RGBA_DXT3 = 0x83F2,					//GL_COMPRESSED_RGBA_S3TC_DXT3_EXT
	RGBA_DXT5 = 0x83F3,					//GL_COMPRESSED_RGBA_S3TC_DXT5_EXT
	R_ATI1N_UNORM = 0x8DBB,				//GL_COMPRESSED_RED_RGTC1
	R_ATI1N_SNORM = 0x8DBC,				//GL_COMPRESSED_SIGNED_RED_RGTC1
	RG_ATI2N_UNORM = 0x8DBD,				//GL_COMPRESSED_RG_RGTC2
	RG_ATI2N_SNORM = 0x8DBE,				//GL_COMPRESSED_SIGNED_RG_RGTC2
	RGB_BP_UNSIGNED_FLOAT = 0x8E8F,		//GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT
	RGB_BP_SIGNED_FLOAT = 0x8E8E,			//GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT
	RGB_BP_UNORM = 0x8E8C,					//GL_COMPRESSED_RGBA_BPTC_UNORM
	RGB_PVRTC_4BPPV1 = 0x8C00,				//GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG
	RGB_PVRTC_2BPPV1 = 0x8C01,				//GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG
	RGBA_PVRTC_4BPPV1 = 0x8C02,			//GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG
	RGBA_PVRTC_2BPPV1 = 0x8C03,			//GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG
	RGBA_PVRTC_4BPPV2 = 0x9137,			//GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG
	RGBA_PVRTC_2BPPV2 = 0x9138,			//GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG
	ATC_RGB = 0x8C92,						//GL_ATC_RGB_AMD
	ATC_RGBA_EXPLICIT_ALPHA = 0x8C93,		//GL_ATC_RGBA_EXPLICIT_ALPHA_AMD
	ATC_RGBA_INTERPOLATED_ALPHA = 0x87EE,	//GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD

	RGB_ETC = 0x8D64,						//GL_COMPRESSED_RGB8_ETC1
	RGB_ETC2 = 0x9274,						//GL_COMPRESSED_RGB8_ETC2
	RGBA_PUNCHTHROUGH_ETC2 = 0x9276,		//GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2
	RGBA_ETC2 = 0x9278,					//GL_COMPRESSED_RGBA8_ETC2_EAC
	R11_EAC = 0x9270,						//GL_COMPRESSED_R11_EAC
	SIGNED_R11_EAC = 0x9271,				//GL_COMPRESSED_SIGNED_R11_EAC
	RG11_EAC = 0x9272,						//GL_COMPRESSED_RG11_EAC
	SIGNED_RG11_EAC = 0x9273,				//GL_COMPRESSED_SIGNED_RG11_EAC

	RGBA_ASTC_4x4 = 0x93B0,				//GL_COMPRESSED_RGBA_ASTC_4x4_KHR
	RGBA_ASTC_5x4 = 0x93B1,				//GL_COMPRESSED_RGBA_ASTC_5x4_KHR
	RGBA_ASTC_5x5 = 0x93B2,				//GL_COMPRESSED_RGBA_ASTC_5x5_KHR
	RGBA_ASTC_6x5 = 0x93B3,				//GL_COMPRESSED_RGBA_ASTC_6x5_KHR
	RGBA_ASTC_6x6 = 0x93B4,				//GL_COMPRESSED_RGBA_ASTC_6x6_KHR
	RGBA_ASTC_8x5 = 0x93B5,				//GL_COMPRESSED_RGBA_ASTC_8x5_KHR
	RGBA_ASTC_8x6 = 0x93B6,				//GL_COMPRESSED_RGBA_ASTC_8x6_KHR
	RGBA_ASTC_8x8 = 0x93B7,				//GL_COMPRESSED_RGBA_ASTC_8x8_KHR
	RGBA_ASTC_10x5 = 0x93B8,				//GL_COMPRESSED_RGBA_ASTC_10x5_KHR
	RGBA_ASTC_10x6 = 0x93B9,				//GL_COMPRESSED_RGBA_ASTC_10x6_KHR
	RGBA_ASTC_10x8 = 0x93BA,				//GL_COMPRESSED_RGBA_ASTC_10x8_KHR
	RGBA_ASTC_10x10 = 0x93BB,				//GL_COMPRESSED_RGBA_ASTC_10x10_KHR
	RGBA_ASTC_12x10 = 0x93BC,				//GL_COMPRESSED_RGBA_ASTC_12x10_KHR
	RGBA_ASTC_12x12 = 0x93BD,				//GL_COMPRESSED_RGBA_ASTC_12x12_KHR

	// sRGB formats
	SRGB_DXT1 = 0x8C4C,					//GL_COMPRESSED_SRGB_S3TC_DXT1_EXT
	SRGB_ALPHA_DXT1 = 0x8C4D,				//GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT
	SRGB_ALPHA_DXT3 = 0x8C4E,				//GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT
	SRGB_ALPHA_DXT5 = 0x8C4F,				//GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT
	SRGB_BP_UNORM = 0x8E8D,				//GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM
	SRGB_PVRTC_2BPPV1 = 0x8A54,			//GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT
	SRGB_PVRTC_4BPPV1 = 0x8A55,			//GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT
	SRGB_ALPHA_PVRTC_2BPPV1 = 0x8A56,		//GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT
	SRGB_ALPHA_PVRTC_4BPPV1 = 0x8A57,		//GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT
	SRGB_ALPHA_PVRTC_2BPPV2 = 0x93F0,		//COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG
	SRGB_ALPHA_PVRTC_4BPPV2 = 0x93F1,		//GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG
	SRGB8_ETC2 = 0x9275,						//GL_COMPRESSED_SRGB8_ETC2
	SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 = 0x9277,	//GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2
	SRGB8_ALPHA8_ETC2_EAC = 0x9279,			//GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC
	SRGB8_ALPHA8_ASTC_4x4 = 0x93D0,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR
	SRGB8_ALPHA8_ASTC_5x4 = 0x93D1,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR
	SRGB8_ALPHA8_ASTC_5x5 = 0x93D2,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR
	SRGB8_ALPHA8_ASTC_6x5 = 0x93D3,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR
	SRGB8_ALPHA8_ASTC_6x6 = 0x93D4,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR
	SRGB8_ALPHA8_ASTC_8x5 = 0x93D5,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR
	SRGB8_ALPHA8_ASTC_8x6 = 0x93D6,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR
	SRGB8_ALPHA8_ASTC_8x8 = 0x93D7,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR
	SRGB8_ALPHA8_ASTC_10x5 = 0x93D8,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR
	SRGB8_ALPHA8_ASTC_10x6 = 0x93D9,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR
	SRGB8_ALPHA8_ASTC_10x8 = 0x93DA,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR
	SRGB8_ALPHA8_ASTC_10x10 = 0x93DB,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR
	SRGB8_ALPHA8_ASTC_12x10 = 0x93DC,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR
	SRGB8_ALPHA8_ASTC_12x12 = 0x93DD,		//GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR

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

enum class GLExternalFormat
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
	DEPTH = 0x1902,			//GL_DEPTH_COMPONENT
	DEPTH_STENCIL = 0x84F9,	//GL_DEPTH_STENCIL
	STENCIL = 0x1901,			//GL_STENCIL_INDEX

	LUMINANCE = 0x1909,				//GL_LUMINANCE
	ALPHA = 0x1906,					//GL_ALPHA
	LUMINANCE_ALPHA = 0x190A,			//GL_LUMINANCE_ALPHA

	SRGB_EXT = 0x8C40,					//SRGB_EXT
	SRGB_ALPHA_EXT = 0x8C42			//SRGB_ALPHA_EXT
};

enum class GLTypeFormat
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
	UINT16_RGB5A1 = 0x8034,		//GL_UNSIGNED_SHORT_5_5_5_1
	UINT16_RGB5A1_REV = 0x8366,	//GL_UNSIGNED_SHORT_1_5_5_5_REV
	UINT16_R5G6B5 = 0x8363,		//GL_UNSIGNED_SHORT_5_6_5
	UINT16_R5G6B5_REV = 0x8364,	//GL_UNSIGNED_SHORT_5_6_5_REV
	UINT16_RGBA4 = 0x8033,			//GL_UNSIGNED_SHORT_4_4_4_4
	UINT16_RGBA4_REV = 0x8365,		//GL_UNSIGNED_SHORT_4_4_4_4_REV
	UINT32_RGBA8 = 0x8035,			//GL_UNSIGNED_SHORT_8_8_8_8
	UINT32_RGBA8_REV = 0x8367,		//GL_UNSIGNED_SHORT_8_8_8_8_REV
	UINT32_RGB10A2 = 0x8036,		//GL_UNSIGNED_INT_10_10_10_2
	UINT32_RGB10A2_REV = 0x8368,	//GL_UNSIGNED_INT_2_10_10_10_REV

	UINT8_RG4_REV_GTC = 0xFFFD,
	UINT16_A1RGB5_GTC = 0xFFFC
};

enum class GLTarget
{
	TARGET_1D = 0x0DE0,
	TARGET_1D_ARRAY = 0x8C18,
	TARGET_2D = 0x0DE1,
	TARGET_2D_ARRAY = 0x8C1A,
	TARGET_3D = 0x806F,
	TARGET_RECT = 0x84F5,
	TARGET_RECT_ARRAY = 0x84F5, // Not supported by OpenGL
	TARGET_CUBE = 0x8513,
	TARGET_CUBE_ARRAY = 0x9009
};

struct FormatDesc
{
	GLInternalFormat	glInternalFormat;
	GLExternalFormat	glFormat;
	GLTypeFormat		glType;
	Format				format;
};

Format MapKTXFormatToEngine(uint32_t internalFormat, uint32_t format, uint32_t type)
{
	static const GLExternalFormat s_externalBGR = GLExternalFormat::BGR;
	static const GLExternalFormat s_externalBGRA = GLExternalFormat::BGRA;
	static const GLExternalFormat s_externalBGRInt = GLExternalFormat::BGR_INTEGER;
	static const GLExternalFormat s_externalBGRAInt = GLExternalFormat::BGRA_INTEGER;

	static const GLExternalFormat s_externalSRGB8 = GLExternalFormat::RGB;
	static const GLExternalFormat s_externalSRGB8_A8 = GLExternalFormat::RGBA;

	static const GLInternalFormat s_internalBGRA = GLInternalFormat::BGRA8_UNORM;
	static const GLInternalFormat s_internalRGBETC = GLInternalFormat::RGB_ETC;

	static const GLInternalFormat s_internalLuminance8 = GLInternalFormat::LUMINANCE8;
	static const GLInternalFormat s_internalAlpha8 = GLInternalFormat::ALPHA8;
	static const GLInternalFormat s_internalLuminanceAlpha8 = GLInternalFormat::LUMINANCE8_ALPHA8;

	static const GLInternalFormat s_internalLuminance16 = GLInternalFormat::LUMINANCE16;
	static const GLInternalFormat s_internalAlpha16 = GLInternalFormat::ALPHA16;
	static const GLInternalFormat s_internalLuminanceAlpha16 = GLInternalFormat::LUMINANCE16_ALPHA16;

	static const GLExternalFormat s_externalLuminance = GLExternalFormat::LUMINANCE;
	static const GLExternalFormat s_externalAlpha = GLExternalFormat::ALPHA;
	static const GLExternalFormat s_externalLuminanceAlpha = GLExternalFormat::LUMINANCE_ALPHA;

	static const FormatDesc table[] =
	{
		{ GLInternalFormat::RG4_EXT, GLExternalFormat::RG, GLTypeFormat::UINT8_RG4_REV_GTC, Format::Unknown },			//FORMAT_R4G4_UNORM,
		{ GLInternalFormat::RGBA4, GLExternalFormat::RGBA, GLTypeFormat::UINT16_RGBA4_REV, Format::Unknown },			//FORMAT_RGBA4_UNORM,
		{ GLInternalFormat::RGBA4, GLExternalFormat::RGBA, GLTypeFormat::UINT16_RGBA4, Format::B4G4R4A4_UNorm },		//FORMAT_BGRA4_UNORM,
		{ GLInternalFormat::R5G6B5, GLExternalFormat::RGB, GLTypeFormat::UINT16_R5G6B5_REV, Format::Unknown },			//FORMAT_R5G6B5_UNORM,
		{ GLInternalFormat::R5G6B5, GLExternalFormat::RGB, GLTypeFormat::UINT16_R5G6B5, Format::B5G6R5_UNorm },			//FORMAT_B5G6R5_UNORM,
		{ GLInternalFormat::RGB5A1, GLExternalFormat::RGBA, GLTypeFormat::UINT16_RGB5A1_REV, Format::Unknown },			//FORMAT_RGB5A1_UNORM,
		{ GLInternalFormat::RGB5A1, GLExternalFormat::RGBA, GLTypeFormat::UINT16_RGB5A1, Format::B5G5R5A1_UNorm },		//FORMAT_BGR5A1_UNORM,
		{ GLInternalFormat::RGB5A1, GLExternalFormat::RGBA, GLTypeFormat::UINT16_A1RGB5_GTC, Format::Unknown },			//FORMAT_A1RGB5_UNORM,

		{ GLInternalFormat::R8_UNORM, GLExternalFormat::RED, GLTypeFormat::U8, Format::R8_UNorm },		//FORMAT_R8_UNORM,
		{ GLInternalFormat::R8_SNORM, GLExternalFormat::RED, GLTypeFormat::I8, Format::R8_SNorm },		//FORMAT_R8_SNORM,
		{ GLInternalFormat::R8_USCALED_GTC, GLExternalFormat::RED, GLTypeFormat::U8, Format::Unknown },	//FORMAT_R8_USCALED,
		{ GLInternalFormat::R8_SSCALED_GTC, GLExternalFormat::RED, GLTypeFormat::I8, Format::Unknown },	//FORMAT_R8_SSCALED,
		{ GLInternalFormat::R8U, GLExternalFormat::RED_INTEGER, GLTypeFormat::U8, Format::R8_UInt },	//FORMAT_R8_UINT,
		{ GLInternalFormat::R8I, GLExternalFormat::RED_INTEGER, GLTypeFormat::I8, Format::R8_SInt },	//FORMAT_R8_SINT,
		{ GLInternalFormat::SR8, GLExternalFormat::RED, GLTypeFormat::U8, Format::Unknown },			//FORMAT_R8_SRGB,

		{ GLInternalFormat::RG8_UNORM, GLExternalFormat::RG, GLTypeFormat::U8, Format::R8G8_UNorm },			//FORMAT_RG8_UNORM,
		{ GLInternalFormat::RG8_SNORM, GLExternalFormat::RG, GLTypeFormat::I8, Format::R8G8_SNorm },			//FORMAT_RG8_SNORM,
		{ GLInternalFormat::RG8_USCALED_GTC, GLExternalFormat::RG, GLTypeFormat::U8, Format::Unknown },			//FORMAT_RG8_USCALED,
		{ GLInternalFormat::RG8_SSCALED_GTC, GLExternalFormat::RG, GLTypeFormat::I8, Format::Unknown },			//FORMAT_RG8_SSCALED,
		{ GLInternalFormat::RG8U, GLExternalFormat::RG_INTEGER, GLTypeFormat::U8, Format::R8G8_UInt },			//FORMAT_RG8_UINT,
		{ GLInternalFormat::RG8I, GLExternalFormat::RG_INTEGER, GLTypeFormat::I8, Format::R8G8_SInt },			//FORMAT_RG8_SINT,
		{ GLInternalFormat::SRG8, GLExternalFormat::RG, GLTypeFormat::U8, Format::Unknown },					//FORMAT_RG8_SRGB,

		{ GLInternalFormat::RGB8_UNORM, GLExternalFormat::RGB, GLTypeFormat::U8, Format::Unknown },				//FORMAT_RGB8_UNORM,
		{ GLInternalFormat::RGB8_SNORM, GLExternalFormat::RGB, GLTypeFormat::I8, Format::Unknown },				//FORMAT_RGB8_SNORM,
		{ GLInternalFormat::RGB8_USCALED_GTC, GLExternalFormat::RGB, GLTypeFormat::U8, Format::Unknown },		//FORMAT_RGB8_USCALED,
		{ GLInternalFormat::RGB8_SSCALED_GTC, GLExternalFormat::RGB, GLTypeFormat::I8, Format::Unknown },		//FORMAT_RGB8_SSCALED,
		{ GLInternalFormat::RGB8U, GLExternalFormat::RGB_INTEGER, GLTypeFormat::U8, Format::Unknown },			//FORMAT_RGB8_UINT,
		{ GLInternalFormat::RGB8I, GLExternalFormat::RGB_INTEGER, GLTypeFormat::I8, Format::Unknown },			//FORMAT_RGB8_SINT,
		{ GLInternalFormat::SRGB8, s_externalSRGB8, GLTypeFormat::U8, Format::Unknown },						//FORMAT_RGB8_SRGB,

		{ GLInternalFormat::RGB8_UNORM, s_externalBGR, GLTypeFormat::U8, Format::Unknown },				//FORMAT_BGR8_UNORM_PACK8,
		{ GLInternalFormat::RGB8_SNORM, s_externalBGR, GLTypeFormat::I8, Format::Unknown },				//FORMAT_BGR8_SNORM_PACK8,
		{ GLInternalFormat::RGB8_USCALED_GTC, s_externalBGR, GLTypeFormat::U8, Format::Unknown },		//FORMAT_BGR8_USCALED_PACK8,
		{ GLInternalFormat::RGB8_SSCALED_GTC, s_externalBGR, GLTypeFormat::I8, Format::Unknown },		//FORMAT_BGR8_SSCALED_PACK8,
		{ GLInternalFormat::RGB8U, s_externalBGRInt, GLTypeFormat::U8, Format::Unknown },				//FORMAT_BGR8_UINT_PACK8,
		{ GLInternalFormat::RGB8I, s_externalBGRInt, GLTypeFormat::I8, Format::Unknown },				//FORMAT_BGR8_SINT_PACK8,
		{ GLInternalFormat::SRGB8, s_externalBGR, GLTypeFormat::U8, Format::Unknown },					//FORMAT_BGR8_SRGB_PACK8,

		{ GLInternalFormat::RGBA8_UNORM, GLExternalFormat::RGBA, GLTypeFormat::U8, Format::R8G8B8A8_UNorm },	//FORMAT_RGBA8_UNORM_PACK8,
		{ GLInternalFormat::RGBA8_SNORM, GLExternalFormat::RGBA, GLTypeFormat::I8, Format::R8G8B8A8_SNorm },	//FORMAT_RGBA8_SNORM_PACK8,
		{ GLInternalFormat::RGBA8_USCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::U8, Format::Unknown },		//FORMAT_RGBA8_USCALED_PACK8,
		{ GLInternalFormat::RGBA8_SSCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::I8, Format::Unknown },		//FORMAT_RGBA8_SSCALED_PACK8,
		{ GLInternalFormat::RGBA8U, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::U8, Format::R8G8B8A8_UInt },	//FORMAT_RGBA8_UINT_PACK8,
		{ GLInternalFormat::RGBA8I, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::I8, Format::R8G8B8A8_SInt },	//FORMAT_RGBA8_SINT_PACK8,
		{ GLInternalFormat::SRGB8_ALPHA8, s_externalSRGB8_A8, GLTypeFormat::U8, Format::R8G8B8A8_UNorm_SRGB },	//FORMAT_RGBA8_SRGB_PACK8,

		{ s_internalBGRA, s_externalBGRA, GLTypeFormat::U8, Format::Unknown },							//FORMAT_BGRA8_UNORM_PACK8,
		{ GLInternalFormat::RGBA8_SNORM, s_externalBGRA, GLTypeFormat::I8, Format::Unknown },			//FORMAT_BGRA8_SNORM_PACK8,
		{ GLInternalFormat::RGBA8_USCALED_GTC, s_externalBGRA, GLTypeFormat::U8, Format::Unknown },		//FORMAT_BGRA8_USCALED_PACK8,
		{ GLInternalFormat::RGBA8_SSCALED_GTC, s_externalBGRA, GLTypeFormat::I8, Format::Unknown },		//FORMAT_BGRA8_SSCALED_PACK8,
		{ GLInternalFormat::RGBA8U, s_externalBGRAInt, GLTypeFormat::U8, Format::Unknown },				//FORMAT_BGRA8_UINT_PACK8,
		{ GLInternalFormat::RGBA8I, s_externalBGRAInt, GLTypeFormat::I8, Format::Unknown },				//FORMAT_BGRA8_SINT_PACK8,
		{ GLInternalFormat::SRGB8_ALPHA8, s_externalBGRA, GLTypeFormat::U8, Format::Unknown },			//FORMAT_BGRA8_SRGB_PACK8,

		{ GLInternalFormat::RGBA8_UNORM, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGBA8_REV, Format::Unknown },			//FORMAT_ABGR8_UNORM_PACK32,
		{ GLInternalFormat::RGBA8_SNORM, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGBA8_REV, Format::Unknown },			//FORMAT_ABGR8_SNORM_PACK32,
		{ GLInternalFormat::RGBA8_USCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGBA8_REV, Format::Unknown },	//FORMAT_ABGR8_USCALED_PACK32,
		{ GLInternalFormat::RGBA8_SSCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGBA8_REV, Format::Unknown },	//FORMAT_ABGR8_SSCALED_PACK32,
		{ GLInternalFormat::RGBA8U, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::UINT32_RGBA8_REV, Format::Unknown },		//FORMAT_ABGR8_UINT_PACK32,
		{ GLInternalFormat::RGBA8I, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::UINT32_RGBA8_REV, Format::Unknown },		//FORMAT_ABGR8_SINT_PACK32,
		{ GLInternalFormat::SRGB8_ALPHA8, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGBA8_REV, Format::Unknown },		//FORMAT_ABGR8_SRGB_PACK32,

		{ GLInternalFormat::RGB10A2_UNORM, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },			//FORMAT_RGB10A2_UNORM_PACK32,
		{ GLInternalFormat::RGB10A2_SNORM_EXT, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },		//FORMAT_RGB10A2_SNORM_PACK32,
		{ GLInternalFormat::RGB10A2_USCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },	//FORMAT_RGB10A2_USCALE_PACK32,
		{ GLInternalFormat::RGB10A2_SSCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },	//FORMAT_RGB10A2_SSCALE_PACK32,
		{ GLInternalFormat::RGB10A2U, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },		//FORMAT_RGB10A2_UINT_PACK32,
		{ GLInternalFormat::RGB10A2I_EXT, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::UINT32_RGB10A2_REV, Format::Unknown },	//FORMAT_RGB10A2_SINT_PACK32,

		{ GLInternalFormat::RGB10A2_UNORM, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGB10A2, Format::R10G10B10A2_UNorm },	//FORMAT_BGR10A2_UNORM_PACK32,
		{ GLInternalFormat::RGB10A2_SNORM_EXT, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGB10A2, Format::Unknown },			//FORMAT_BGR10A2_SNORM_PACK32,
		{ GLInternalFormat::RGB10A2_USCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGB10A2, Format::Unknown },		//FORMAT_BGR10A2_USCALE_PACK32,
		{ GLInternalFormat::RGB10A2_SSCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::UINT32_RGB10A2, Format::Unknown },		//FORMAT_BGR10A2_SSCALE_PACK32,
		{ GLInternalFormat::RGB10A2U, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::UINT32_RGB10A2, Format::Unknown },			//FORMAT_BGR10A2_UINT_PACK32,
		{ GLInternalFormat::RGB10A2I_EXT, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::UINT32_RGB10A2, Format::Unknown },		//FORMAT_BGR10A2_SINT_PACK32,

		{ GLInternalFormat::R16_UNORM, GLExternalFormat::RED, GLTypeFormat::U16, Format::R16_UNorm },			//FORMAT_R16_UNORM_PACK16,
		{ GLInternalFormat::R16_SNORM, GLExternalFormat::RED, GLTypeFormat::I16, Format::R16_SNorm },			//FORMAT_R16_SNORM_PACK16,
		{ GLInternalFormat::R16_USCALED_GTC, GLExternalFormat::RED, GLTypeFormat::U16, Format::Unknown },		//FORMAT_R16_USCALED_PACK16,
		{ GLInternalFormat::R16_SSCALED_GTC, GLExternalFormat::RED, GLTypeFormat::I16, Format::Unknown },		//FORMAT_R16_SSCALED_PACK16,
		{ GLInternalFormat::R16U, GLExternalFormat::RED_INTEGER, GLTypeFormat::U16, Format::R16_UInt },			//FORMAT_R16_UINT_PACK16,
		{ GLInternalFormat::R16I, GLExternalFormat::RED_INTEGER, GLTypeFormat::I16, Format::R16_SInt },			//FORMAT_R16_SINT_PACK16,
		{ GLInternalFormat::R16F, GLExternalFormat::RED, GLTypeFormat::F16, Format::R16_Float },				//FORMAT_R16_SFLOAT_PACK16,

		{ GLInternalFormat::RG16_UNORM, GLExternalFormat::RG, GLTypeFormat::U16, Format::R16G16_UNorm },		//FORMAT_RG16_UNORM_PACK16,
		{ GLInternalFormat::RG16_SNORM, GLExternalFormat::RG, GLTypeFormat::I16, Format::R16G16_SNorm },		//FORMAT_RG16_SNORM_PACK16,
		{ GLInternalFormat::RG16_USCALED_GTC, GLExternalFormat::RG, GLTypeFormat::U16, Format::Unknown },		//FORMAT_RG16_USCALED_PACK16,
		{ GLInternalFormat::RG16_SSCALED_GTC, GLExternalFormat::RG, GLTypeFormat::I16, Format::Unknown },		//FORMAT_RG16_SSCALED_PACK16,
		{ GLInternalFormat::RG16U, GLExternalFormat::RG_INTEGER, GLTypeFormat::U16, Format::R16G16_UInt },		//FORMAT_RG16_UINT_PACK16,
		{ GLInternalFormat::RG16I, GLExternalFormat::RG_INTEGER, GLTypeFormat::I16, Format::R16G16_SInt },		//FORMAT_RG16_SINT_PACK16,
		{ GLInternalFormat::RG16F, GLExternalFormat::RG, GLTypeFormat::F16, Format::R16G16_Float },				//FORMAT_RG16_SFLOAT_PACK16,

		{ GLInternalFormat::RGB16_UNORM, GLExternalFormat::RGB, GLTypeFormat::U16, Format::Unknown },			//FORMAT_RGB16_UNORM_PACK16,
		{ GLInternalFormat::RGB16_SNORM, GLExternalFormat::RGB, GLTypeFormat::I16, Format::Unknown },			//FORMAT_RGB16_SNORM_PACK16,
		{ GLInternalFormat::RGB16_USCALED_GTC, GLExternalFormat::RGB, GLTypeFormat::U16, Format::Unknown },		//FORMAT_RGB16_USCALED_PACK16,
		{ GLInternalFormat::RGB16_SSCALED_GTC, GLExternalFormat::RGB, GLTypeFormat::I16, Format::Unknown },		//FORMAT_RGB16_USCALED_PACK16,
		{ GLInternalFormat::RGB16U, GLExternalFormat::RGB_INTEGER, GLTypeFormat::U16, Format::Unknown },		//FORMAT_RGB16_UINT_PACK16,
		{ GLInternalFormat::RGB16I, GLExternalFormat::RGB_INTEGER, GLTypeFormat::I16, Format::Unknown },		//FORMAT_RGB16_SINT_PACK16,
		{ GLInternalFormat::RGB16F, GLExternalFormat::RGB, GLTypeFormat::F16, Format::Unknown },				//FORMAT_RGB16_SFLOAT_PACK16,

		{ GLInternalFormat::RGBA16_UNORM, GLExternalFormat::RGBA, GLTypeFormat::U16, Format::R16G16B16A16_UNorm },		//FORMAT_RGBA16_UNORM_PACK16,
		{ GLInternalFormat::RGBA16_SNORM, GLExternalFormat::RGBA, GLTypeFormat::I16, Format::R16G16B16A16_SNorm },		//FORMAT_RGBA16_SNORM_PACK16,
		{ GLInternalFormat::RGBA16_USCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::U16, Format::Unknown },			//FORMAT_RGBA16_USCALED_PACK16,
		{ GLInternalFormat::RGBA16_SSCALED_GTC, GLExternalFormat::RGBA, GLTypeFormat::I16, Format::Unknown },			//FORMAT_RGBA16_SSCALED_PACK16,
		{ GLInternalFormat::RGBA16U, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::U16, Format::R16G16B16A16_UInt },	//FORMAT_RGBA16_UINT_PACK16,
		{ GLInternalFormat::RGBA16I, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::I16, Format::R16G16B16A16_SInt },	//FORMAT_RGBA16_SINT_PACK16,
		{ GLInternalFormat::RGBA16F, GLExternalFormat::RGBA, GLTypeFormat::F16, Format::R16G16B16A16_Float },			//FORMAT_RGBA16_SFLOAT_PACK16,

		{ GLInternalFormat::R32U, GLExternalFormat::RED_INTEGER, GLTypeFormat::U32, Format::R32_UInt },					//FORMAT_R32_UINT_PACK32,
		{ GLInternalFormat::R32I, GLExternalFormat::RED_INTEGER, GLTypeFormat::I32, Format::R32_SInt },					//FORMAT_R32_SINT_PACK32,
		{ GLInternalFormat::R32F, GLExternalFormat::RED, GLTypeFormat::F32, Format::R32_Float },						//FORMAT_R32_SFLOAT_PACK32,

		{ GLInternalFormat::RG32U, GLExternalFormat::RG_INTEGER, GLTypeFormat::U32, Format::R32G32_UInt },				//FORMAT_RG32_UINT_PACK32,
		{ GLInternalFormat::RG32I, GLExternalFormat::RG_INTEGER, GLTypeFormat::I32, Format::R32G32_SInt },				//FORMAT_RG32_SINT_PACK32,
		{ GLInternalFormat::RG32F, GLExternalFormat::RG, GLTypeFormat::F32, Format::R32G32_Float },						//FORMAT_RG32_SFLOAT_PACK32,

		{ GLInternalFormat::RGB32U, GLExternalFormat::RGB_INTEGER, GLTypeFormat::U32, Format::R32G32B32_UInt },			//FORMAT_RGB32_UINT_PACK32,
		{ GLInternalFormat::RGB32I, GLExternalFormat::RGB_INTEGER, GLTypeFormat::I32, Format::R32G32B32_SInt },			//FORMAT_RGB32_SINT_PACK32,
		{ GLInternalFormat::RGB32F, GLExternalFormat::RGB, GLTypeFormat::F32, Format::R32G32B32_Float },				//FORMAT_RGB32_SFLOAT_PACK32,

		{ GLInternalFormat::RGBA32U, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::U32, Format::R32G32B32A32_UInt },	//FORMAT_RGBA32_UINT_PACK32,
		{ GLInternalFormat::RGBA32I, GLExternalFormat::RGBA_INTEGER, GLTypeFormat::I32, Format::R32G32B32A32_SInt },	//FORMAT_RGBA32_SINT_PACK32,
		{ GLInternalFormat::RGBA32F, GLExternalFormat::RGBA, GLTypeFormat::F32, Format::R32G32B32A32_Float },			//FORMAT_RGBA32_SFLOAT_PACK32,

		{ GLInternalFormat::R64F_EXT, GLExternalFormat::RED, GLTypeFormat::U64, Format::Unknown },			//FORMAT_R64_UINT_PACK64,
		{ GLInternalFormat::R64F_EXT, GLExternalFormat::RED, GLTypeFormat::I64, Format::Unknown },			//FORMAT_R64_SINT_PACK64,
		{ GLInternalFormat::R64F_EXT, GLExternalFormat::RED, GLTypeFormat::F64, Format::Unknown },			//FORMAT_R64_SFLOAT_PACK64,

		{ GLInternalFormat::RG64F_EXT, GLExternalFormat::RG, GLTypeFormat::U64, Format::Unknown },			//FORMAT_RG64_UINT_PACK64,
		{ GLInternalFormat::RG64F_EXT, GLExternalFormat::RG, GLTypeFormat::I64, Format::Unknown },			//FORMAT_RG64_SINT_PACK64,
		{ GLInternalFormat::RG64F_EXT, GLExternalFormat::RG, GLTypeFormat::F64, Format::Unknown },			//FORMAT_RG64_SFLOAT_PACK64,

		{ GLInternalFormat::RGB64F_EXT, GLExternalFormat::RGB, GLTypeFormat::U64, Format::Unknown },		//FORMAT_RGB64_UINT_PACK64,
		{ GLInternalFormat::RGB64F_EXT, GLExternalFormat::RGB, GLTypeFormat::I64, Format::Unknown },		//FORMAT_RGB64_SINT_PACK64,
		{ GLInternalFormat::RGB64F_EXT, GLExternalFormat::RGB, GLTypeFormat::F64, Format::Unknown },		//FORMAT_RGB64_SFLOAT_PACK64,

		{ GLInternalFormat::RGBA64F_EXT, GLExternalFormat::RGBA, GLTypeFormat::U64, Format::Unknown },		//FORMAT_RGBA64_UINT_PACK64,
		{ GLInternalFormat::RGBA64F_EXT, GLExternalFormat::RGBA, GLTypeFormat::I64, Format::Unknown },		//FORMAT_RGBA64_SINT_PACK64,
		{ GLInternalFormat::RGBA64F_EXT, GLExternalFormat::RGBA, GLTypeFormat::F64, Format::Unknown },		//FORMAT_RGBA64_SFLOAT_PACK64,

		{ GLInternalFormat::RG11B10F, GLExternalFormat::RGB, GLTypeFormat::UINT32_RG11B10F_REV, Format::R11G11B10_Float },	//FORMAT_RG11B10_UFLOAT_PACK32,
		{ GLInternalFormat::RGB9E5, GLExternalFormat::RGB, GLTypeFormat::UINT32_RGB9_E5_REV, Format::R9G9B9E5_Float },		//FORMAT_RGB9E5_UFLOAT_PACK32,

		{ GLInternalFormat::D16, GLExternalFormat::DEPTH, GLTypeFormat::NONE, Format::D16_UNorm },							//FORMAT_D16_UNORM_PACK16,
		{ GLInternalFormat::D24, GLExternalFormat::DEPTH, GLTypeFormat::NONE, Format::Unknown },							//FORMAT_D24_UNORM,
		{ GLInternalFormat::D32F, GLExternalFormat::DEPTH, GLTypeFormat::NONE, Format::D32_Float },							//FORMAT_D32_UFLOAT,
		{ GLInternalFormat::S8_EXT, GLExternalFormat::STENCIL, GLTypeFormat::NONE, Format::R8_UNorm },						//FORMAT_S8_UNORM,
		{ GLInternalFormat::D16S8_EXT, GLExternalFormat::DEPTH, GLTypeFormat::NONE, Format::Unknown },						//FORMAT_D16_UNORM_S8_UINT_PACK32,
		{ GLInternalFormat::D24S8, GLExternalFormat::DEPTH_STENCIL, GLTypeFormat::NONE, Format::D24S8 },					//FORMAT_D24_UNORM_S8_UINT_PACK32,
		{ GLInternalFormat::D32FS8X24, GLExternalFormat::DEPTH_STENCIL, GLTypeFormat::NONE, Format::D32_Float_S8_UInt },	//FORMAT_D32_SFLOAT_S8_UINT_PACK64,

		{ GLInternalFormat::RGB_DXT1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_DXT1_UNORM_BLOCK8,
		{ GLInternalFormat::SRGB_DXT1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_DXT1_SRGB_BLOCK8,
		{ GLInternalFormat::RGBA_DXT1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_DXT1_UNORM_BLOCK8,
		{ GLInternalFormat::SRGB_ALPHA_DXT1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_DXT1_SRGB_BLOCK8,
		{ GLInternalFormat::RGBA_DXT3, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_DXT3_UNORM_BLOCK16,
		{ GLInternalFormat::SRGB_ALPHA_DXT3, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_DXT3_SRGB_BLOCK16,
		{ GLInternalFormat::RGBA_DXT5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_DXT5_UNORM_BLOCK16,
		{ GLInternalFormat::SRGB_ALPHA_DXT5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_DXT5_SRGB_BLOCK16,
		{ GLInternalFormat::R_ATI1N_UNORM, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_R_ATI1N_UNORM_BLOCK8,
		{ GLInternalFormat::R_ATI1N_SNORM, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_R_ATI1N_SNORM_BLOCK8,
		{ GLInternalFormat::RG_ATI2N_UNORM, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RG_ATI2N_UNORM_BLOCK16,
		{ GLInternalFormat::RG_ATI2N_SNORM, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RG_ATI2N_SNORM_BLOCK16,
		{ GLInternalFormat::RGB_BP_UNSIGNED_FLOAT, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },	//FORMAT_RGB_BP_UFLOAT_BLOCK16,
		{ GLInternalFormat::RGB_BP_SIGNED_FLOAT, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },		//FORMAT_RGB_BP_SFLOAT_BLOCK16,
		{ GLInternalFormat::RGB_BP_UNORM, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGB_BP_UNORM,
		{ GLInternalFormat::SRGB_BP_UNORM, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGB_BP_SRGB,

		{ s_internalRGBETC, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },									//FORMAT_RGB_ETC2_UNORM_BLOCK8,
		{ GLInternalFormat::SRGB8_ETC2, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },						//FORMAT_RGB_ETC2_SRGB_BLOCK8,
		{ GLInternalFormat::RGBA_PUNCHTHROUGH_ETC2, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ETC2_PUNCHTHROUGH_UNORM,
		{ GLInternalFormat::SRGB8_PUNCHTHROUGH_ALPHA1_ETC2, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },	//FORMAT_RGBA_ETC2_PUNCHTHROUGH_SRGB,
		{ GLInternalFormat::RGBA_ETC2, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },						//FORMAT_RGBA_ETC2_UNORM_BLOCK16,
		{ GLInternalFormat::SRGB8_ALPHA8_ETC2_EAC, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ETC2_SRGB_BLOCK16,
		{ GLInternalFormat::R11_EAC, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },							//FORMAT_R11_EAC_UNORM,
		{ GLInternalFormat::SIGNED_R11_EAC, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_R11_EAC_SNORM,
		{ GLInternalFormat::RG11_EAC, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },						//FORMAT_RG11_EAC_UNORM,
		{ GLInternalFormat::SIGNED_RG11_EAC, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RG11_EAC_SNORM,

		{ GLInternalFormat::RGBA_ASTC_4x4, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC4X4_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_4x4, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC4X4_SRGB,
		{ GLInternalFormat::RGBA_ASTC_5x4, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC5X4_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_5x4, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC5X4_SRGB,
		{ GLInternalFormat::RGBA_ASTC_5x5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC5X5_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_5x5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC5X5_SRGB,
		{ GLInternalFormat::RGBA_ASTC_6x5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC6X5_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_6x5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC6X5_SRGB,
		{ GLInternalFormat::RGBA_ASTC_6x6, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC6X6_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_6x6, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC6X6_SRGB,
		{ GLInternalFormat::RGBA_ASTC_8x5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC8X5_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_8x5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC8X5_SRGB,
		{ GLInternalFormat::RGBA_ASTC_8x6, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC8X6_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_8x6, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC8X6_SRGB,
		{ GLInternalFormat::RGBA_ASTC_8x8, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC8X8_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_8x8, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC8X8_SRGB,
		{ GLInternalFormat::RGBA_ASTC_10x5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC10X5_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_10x5, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC10X5_SRGB,
		{ GLInternalFormat::RGBA_ASTC_10x6, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC10X6_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_10x6, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC10X6_SRGB,
		{ GLInternalFormat::RGBA_ASTC_10x8, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC10X8_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_10x8, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC10X8_SRGB,
		{ GLInternalFormat::RGBA_ASTC_10x10, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC10X10_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_10x10, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC10X10_SRGB,
		{ GLInternalFormat::RGBA_ASTC_12x10, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC12X10_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_12x10, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC12X10_SRGB,
		{ GLInternalFormat::RGBA_ASTC_12x12, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },					//FORMAT_RGBA_ASTC12X12_UNORM,
		{ GLInternalFormat::SRGB8_ALPHA8_ASTC_12x12, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ASTC12X12_SRGB,

		{ GLInternalFormat::RGB_PVRTC_4BPPV1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_PVRTC1_8X8_UNORM_BLOCK32,
		{ GLInternalFormat::SRGB_PVRTC_2BPPV1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_PVRTC1_8X8_SRGB_BLOCK32,
		{ GLInternalFormat::RGB_PVRTC_2BPPV1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_PVRTC1_16X8_UNORM_BLOCK32,
		{ GLInternalFormat::SRGB_PVRTC_4BPPV1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGB_PVRTC1_16X8_SRGB_BLOCK32,
		{ GLInternalFormat::RGBA_PVRTC_4BPPV1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_PVRTC1_8X8_UNORM_BLOCK32,
		{ GLInternalFormat::SRGB_ALPHA_PVRTC_2BPPV1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_PVRTC1_8X8_SRGB_BLOCK32,
		{ GLInternalFormat::RGBA_PVRTC_2BPPV1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_PVRTC1_16X8_UNORM_BLOCK32,
		{ GLInternalFormat::SRGB_ALPHA_PVRTC_4BPPV1, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_PVRTC1_16X8_SRGB_BLOCK32,
		{ GLInternalFormat::RGBA_PVRTC_4BPPV2, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_PVRTC2_4X4_UNORM_BLOCK8,
		{ GLInternalFormat::SRGB_ALPHA_PVRTC_4BPPV2, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_PVRTC2_4X4_SRGB_BLOCK8,
		{ GLInternalFormat::RGBA_PVRTC_2BPPV2, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },				//FORMAT_RGBA_PVRTC2_8X4_UNORM_BLOCK8,
		{ GLInternalFormat::SRGB_ALPHA_PVRTC_2BPPV2, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_PVRTC2_8X4_SRGB_BLOCK8,

		{ GLInternalFormat::RGB_ETC, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },							//FORMAT_RGB_ETC_UNORM_BLOCK8,
		{ GLInternalFormat::ATC_RGB, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },							//FORMAT_RGB_ATC_UNORM_BLOCK8,
		{ GLInternalFormat::ATC_RGBA_EXPLICIT_ALPHA, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },			//FORMAT_RGBA_ATCA_UNORM_BLOCK16,
		{ GLInternalFormat::ATC_RGBA_INTERPOLATED_ALPHA, GLExternalFormat::NONE, GLTypeFormat::NONE, Format::Unknown },		//FORMAT_RGBA_ATCI_UNORM_BLOCK16,

		{ s_internalLuminance8, s_externalLuminance, GLTypeFormat::U8, Format::Unknown },						//FORMAT_L8_UNORM_PACK8,
		{ s_internalAlpha8, s_externalAlpha, GLTypeFormat::U8, Format::Unknown },								//FORMAT_A8_UNORM_PACK8,
		{ s_internalLuminanceAlpha8, s_externalLuminanceAlpha, GLTypeFormat::U8, Format::Unknown },				//FORMAT_LA8_UNORM_PACK8,
		{ s_internalLuminance16, s_externalLuminance, GLTypeFormat::U16, Format::Unknown },						//FORMAT_L16_UNORM_PACK16,
		{ s_internalAlpha16, s_externalAlpha, GLTypeFormat::U16, Format::Unknown },								//FORMAT_A16_UNORM_PACK16,
		{ s_internalLuminanceAlpha16, s_externalLuminanceAlpha, GLTypeFormat::U16, Format::Unknown },			//FORMAT_LA16_UNORM_PACK16,

		{ GLInternalFormat::RGB8_UNORM, s_externalBGRA, GLTypeFormat::U8, Format::Unknown },					//FORMAT_BGRX8_UNORM,
		{ GLInternalFormat::SRGB8, s_externalBGRA, GLTypeFormat::U8, Format::Unknown },							//FORMAT_BGRX8_SRGB,

		{ GLInternalFormat::RG3B2, GLExternalFormat::RGB, GLTypeFormat::UINT8_RG3B2_REV, Format::Unknown },		//FORMAT_RG3B2_UNORM,
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

inline GLTarget GetTarget(const KTXHeader10& header)
{
	if (header.numberOfFaces > 1)
	{
		if (header.numberOfArrayElements > 0)
		{
			return GLTarget::TARGET_CUBE_ARRAY;
		}
		else
		{
			return GLTarget::TARGET_CUBE;
		}
	}
	else if (header.numberOfArrayElements > 0)
	{
		if (header.pixelHeight == 0)
		{
			return GLTarget::TARGET_1D_ARRAY;
		}
		else
		{
			return GLTarget::TARGET_2D_ARRAY;
		}
	}
	else if (header.pixelHeight == 0)
	{
		return GLTarget::TARGET_1D;
	}
	else if (header.pixelDepth > 0)
	{
		return GLTarget::TARGET_3D;
	}
	else
	{
		return GLTarget::TARGET_2D;
	}
}


void GetSurfaceInfo(size_t width,
	size_t height,
	DXGI_FORMAT fmt,
	size_t* outNumBytes,
	size_t* outRowBytes,
	size_t* outNumRows)
{
	size_t numBytes = 0;
	size_t rowBytes = 0;
	size_t numRows = 0;

	bool bc = false;
	bool packed = false;
	bool planar = false;
	size_t bpe = 0;
	switch (fmt)
	{
	case DXGI_FORMAT_BC1_TYPELESS:
	case DXGI_FORMAT_BC1_UNORM:
	case DXGI_FORMAT_BC1_UNORM_SRGB:
	case DXGI_FORMAT_BC4_TYPELESS:
	case DXGI_FORMAT_BC4_UNORM:
	case DXGI_FORMAT_BC4_SNORM:
		bc = true;
		bpe = 8;
		break;

	case DXGI_FORMAT_BC2_TYPELESS:
	case DXGI_FORMAT_BC2_UNORM:
	case DXGI_FORMAT_BC2_UNORM_SRGB:
	case DXGI_FORMAT_BC3_TYPELESS:
	case DXGI_FORMAT_BC3_UNORM:
	case DXGI_FORMAT_BC3_UNORM_SRGB:
	case DXGI_FORMAT_BC5_TYPELESS:
	case DXGI_FORMAT_BC5_UNORM:
	case DXGI_FORMAT_BC5_SNORM:
	case DXGI_FORMAT_BC6H_TYPELESS:
	case DXGI_FORMAT_BC6H_UF16:
	case DXGI_FORMAT_BC6H_SF16:
	case DXGI_FORMAT_BC7_TYPELESS:
	case DXGI_FORMAT_BC7_UNORM:
	case DXGI_FORMAT_BC7_UNORM_SRGB:
		bc = true;
		bpe = 16;
		break;

	case DXGI_FORMAT_R8G8_B8G8_UNORM:
	case DXGI_FORMAT_G8R8_G8B8_UNORM:
	case DXGI_FORMAT_YUY2:
		packed = true;
		bpe = 4;
		break;

	case DXGI_FORMAT_Y210:
	case DXGI_FORMAT_Y216:
		packed = true;
		bpe = 8;
		break;

	case DXGI_FORMAT_NV12:
	case DXGI_FORMAT_420_OPAQUE:
		planar = true;
		bpe = 2;
		break;

	case DXGI_FORMAT_P010:
	case DXGI_FORMAT_P016:
		planar = true;
		bpe = 4;
		break;

	}

	if (bc)
	{
		size_t numBlocksWide = 0;
		if (width > 0)
		{
			numBlocksWide = std::max<size_t>(1, (width + 3) / 4);
		}
		size_t numBlocksHigh = 0;
		if (height > 0)
		{
			numBlocksHigh = std::max<size_t>(1, (height + 3) / 4);
		}
		rowBytes = numBlocksWide * bpe;
		numRows = numBlocksHigh;
		numBytes = rowBytes * numBlocksHigh;
	}
	else if (packed)
	{
		rowBytes = ((width + 1) >> 1) * bpe;
		numRows = height;
		numBytes = rowBytes * height;
	}
	else if (fmt == DXGI_FORMAT_NV11)
	{
		rowBytes = ((width + 3) >> 2) * 4;
		numRows = height * 2; // Direct3D makes this simplifying assumption, although it is larger than the 4:1:1 data
		numBytes = rowBytes * numRows;
	}
	else if (planar)
	{
		rowBytes = ((width + 1) >> 1) * bpe;
		numBytes = (rowBytes * height) + ((rowBytes * height + 1) >> 1);
		numRows = height + ((height + 1) >> 1);
	}
	else
	{
		size_t bpp = BitsPerPixel(MapDXGIFormatToEngine(fmt));
		rowBytes = (width * bpp + 7) / 8; // round up to nearest byte
		numRows = height;
		numBytes = rowBytes * height;
	}

	if (outNumBytes)
	{
		*outNumBytes = numBytes;
	}
	if (outRowBytes)
	{
		*outRowBytes = rowBytes;
	}
	if (outNumRows)
	{
		*outNumRows = numRows;
	}
}


HRESULT FillInitData(size_t width,
	size_t height,
	size_t depth,
	size_t mipCount,
	size_t arraySize,
	DXGI_FORMAT format,
	size_t maxsize,
	size_t bitSize,
	const uint8_t* bitData,
	size_t& twidth,
	size_t& theight,
	size_t& tdepth,
	size_t& skipMip,
	D3D12_SUBRESOURCE_DATA* initData)
{
	if (!bitData || !initData)
	{
		return E_POINTER;
	}

	skipMip = 0;
	twidth = 0;
	theight = 0;
	tdepth = 0;

	size_t numBytes = 0;
	size_t rowBytes = 0;
	const uint8_t* pSrcBits = bitData;
	const uint8_t* pEndBits = bitData + bitSize;

	size_t index = 0;
	for (size_t j = 0; j < arraySize; j++)
	{
		size_t w = width;
		size_t h = height;
		size_t d = depth;
		for (size_t i = 0; i < mipCount; i++)
		{
			GetSurfaceInfo(w,
				h,
				format,
				&numBytes,
				&rowBytes,
				nullptr
			);

			if ((mipCount <= 1) || !maxsize || (w <= maxsize && h <= maxsize && d <= maxsize))
			{
				if (!twidth)
				{
					twidth = w;
					theight = h;
					tdepth = d;
				}

				assert(index < mipCount * arraySize);
				initData[index].pData = (const void*)pSrcBits;
				initData[index].RowPitch = static_cast<UINT>(rowBytes);
				initData[index].SlicePitch = static_cast<UINT>(numBytes);
				++index;
			}
			else if (!j)
			{
				// Count number of skipped mipmaps (first item only)
				++skipMip;
			}

			if (pSrcBits + (numBytes*d) > pEndBits)
			{
				return HRESULT_FROM_WIN32(ERROR_HANDLE_EOF);
			}

			pSrcBits += numBytes * d;

			w = w >> 1;
			h = h >> 1;
			d = d >> 1;
			if (w == 0)
			{
				w = 1;
			}
			if (h == 0)
			{
				h = 1;
			}
			if (d == 0)
			{
				d = 1;
			}
		}
	}

	return (index > 0) ? S_OK : E_FAIL;
}


DXGI_FORMAT MakeSRGB(DXGI_FORMAT format)
{
	switch (format)
	{
	case DXGI_FORMAT_R8G8B8A8_UNORM:
		return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;

	case DXGI_FORMAT_BC1_UNORM:
		return DXGI_FORMAT_BC1_UNORM_SRGB;

	case DXGI_FORMAT_BC2_UNORM:
		return DXGI_FORMAT_BC2_UNORM_SRGB;

	case DXGI_FORMAT_BC3_UNORM:
		return DXGI_FORMAT_BC3_UNORM_SRGB;

	case DXGI_FORMAT_B8G8R8A8_UNORM:
		return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;

	case DXGI_FORMAT_B8G8R8X8_UNORM:
		return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;

	case DXGI_FORMAT_BC7_UNORM:
		return DXGI_FORMAT_BC7_UNORM_SRGB;

	default:
		return format;
	}
}


HRESULT CreateD3DResources(ID3D12Device* d3dDevice,
	uint32_t resDim,
	size_t width,
	size_t height,
	size_t depth,
	size_t mipCount,
	size_t arraySize,
	DXGI_FORMAT format,
	bool forceSRGB,
	bool isCubeMap,
	ID3D12Resource** texture,
	D3D12_CPU_DESCRIPTOR_HANDLE textureView)
{
	if (!d3dDevice)
		return E_POINTER;

	HRESULT hr = E_FAIL;

	if (forceSRGB)
	{
		format = MakeSRGB(format);
	}

	D3D12_HEAP_PROPERTIES HeapProps;
	HeapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
	HeapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	HeapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	HeapProps.CreationNodeMask = 1;
	HeapProps.VisibleNodeMask = 1;

	D3D12_RESOURCE_DESC ResourceDesc;
	ResourceDesc.Alignment = 0;
	ResourceDesc.Width = static_cast<UINT64>(width);
	ResourceDesc.Height = static_cast<UINT>(height);
	ResourceDesc.DepthOrArraySize = static_cast<UINT16>(arraySize);
	ResourceDesc.MipLevels = static_cast<UINT16>(mipCount);
	ResourceDesc.Format = format;
	ResourceDesc.SampleDesc.Count = 1;
	ResourceDesc.SampleDesc.Quality = 0;
	ResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	ResourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	switch (resDim)
	{
	case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
	{
		ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;

		ID3D12Resource* tex = nullptr;
		hr = d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, MY_IID_PPV_ARGS(&tex));

		if (SUCCEEDED(hr) && tex != nullptr)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Format = format;
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			if (arraySize > 1)
			{
				SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
				SRVDesc.Texture1DArray.MipLevels = (!mipCount) ? -1 : ResourceDesc.MipLevels;
				SRVDesc.Texture1DArray.ArraySize = static_cast<UINT>(arraySize);
			}
			else
			{
				SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
				SRVDesc.Texture1D.MipLevels = (!mipCount) ? -1 : ResourceDesc.MipLevels;
			}

			d3dDevice->CreateShaderResourceView(tex, &SRVDesc, textureView);

			if (texture != nullptr)
			{
				*texture = tex;
			}
			else
			{
				tex->SetName(L"KTXTextureLoader");
				tex->Release();
			}
		}
	}
	break;

	case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
	{
		ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;

		ID3D12Resource* tex = nullptr;
		hr = d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, MY_IID_PPV_ARGS(&tex));

		if (SUCCEEDED(hr) && tex != 0)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Format = format;
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			if (isCubeMap)
			{
				if (arraySize > 6)
				{
					SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
					SRVDesc.TextureCubeArray.MipLevels = (!mipCount) ? -1 : ResourceDesc.MipLevels;

					// Earlier we set arraySize to (NumCubes * 6)
					SRVDesc.TextureCubeArray.NumCubes = static_cast<UINT>(arraySize / 6);
				}
				else
				{
					SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
					SRVDesc.TextureCube.MipLevels = (!mipCount) ? -1 : ResourceDesc.MipLevels;
				}
			}
			else if (arraySize > 1)
			{
				SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
				SRVDesc.Texture2DArray.MipLevels = (!mipCount) ? -1 : ResourceDesc.MipLevels;
				SRVDesc.Texture2DArray.ArraySize = static_cast<UINT>(arraySize);
			}
			else
			{
				SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
				SRVDesc.Texture2D.MipLevels = (!mipCount) ? -1 : ResourceDesc.MipLevels;
				SRVDesc.Texture2D.MostDetailedMip = 0;
			}

			d3dDevice->CreateShaderResourceView(tex, &SRVDesc, textureView);

			if (texture != nullptr)
			{
				*texture = tex;
			}
			else
			{
				tex->SetName(L"KTXTextureLoader");
				tex->Release();
			}
		}
	}
	break;

	case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
	{
		ResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		ResourceDesc.DepthOrArraySize = static_cast<UINT16>(depth);

		ID3D12Resource* tex = nullptr;
		hr = d3dDevice->CreateCommittedResource(&HeapProps, D3D12_HEAP_FLAG_NONE, &ResourceDesc,
			D3D12_RESOURCE_STATE_COPY_DEST, nullptr, MY_IID_PPV_ARGS(&tex));

		if (SUCCEEDED(hr) && tex != nullptr)
		{
			D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc = {};
			SRVDesc.Format = format;
			SRVDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

			SRVDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
			SRVDesc.Texture3D.MipLevels = (!mipCount) ? -1 : ResourceDesc.MipLevels;
			SRVDesc.Texture3D.MostDetailedMip = 0;

			d3dDevice->CreateShaderResourceView(tex, &SRVDesc, textureView);

			if (texture != nullptr)
			{
				*texture = tex;
			}
			else
			{
				tex->SetName(L"KTX Texture (3D)");
				tex->Release();
			}
		}
	}
	break;
	}

	return hr;
}

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
	HRESULT hr = S_OK;

	assert(ktxData && (ktxDataSize >= sizeof(KTXHeader10)));

	// Check the KTX magic number
	if (memcmp(ktxData, FOURCC_KTX10, sizeof(FOURCC_KTX10)) != 0)
	{
		HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
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

	auto target = GetTarget(header);
	bool isCubeMap = (target == GLTarget::TARGET_CUBE || target == GLTarget::TARGET_CUBE_ARRAY);

	// Bound sizes (for security purposes we don't trust DDS file metadata larger than the D3D 11.x hardware requirements)
	if (numMips > D3D12_REQ_MIP_LEVELS)
	{
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	D3D12_RESOURCE_DIMENSION resDim;

	switch (target)
	{
	case GLTarget::TARGET_1D:
	case GLTarget::TARGET_1D_ARRAY:
		resDim = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
		if ((header.numberOfArrayElements > D3D12_REQ_TEXTURE1D_ARRAY_AXIS_DIMENSION) ||
			(header.pixelWidth > D3D12_REQ_TEXTURE1D_U_DIMENSION))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	case GLTarget::TARGET_2D:
	case GLTarget::TARGET_2D_ARRAY:
		resDim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		if ((arraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
			(header.pixelWidth > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION) ||
			(header.pixelHeight > D3D12_REQ_TEXTURE2D_U_OR_V_DIMENSION))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	case GLTarget::TARGET_CUBE:
	case GLTarget::TARGET_CUBE_ARRAY:
		resDim = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		// This is the right bound because we set arraySize to (NumCubes*6) above
		if ((arraySize > D3D12_REQ_TEXTURE2D_ARRAY_AXIS_DIMENSION) ||
			(header.pixelWidth > D3D12_REQ_TEXTURECUBE_DIMENSION) ||
			(header.pixelHeight > D3D12_REQ_TEXTURECUBE_DIMENSION))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	case GLTarget::TARGET_3D:
		resDim = D3D12_RESOURCE_DIMENSION_TEXTURE3D;
		if ((arraySize > 1) ||
			(header.pixelWidth > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
			(header.pixelHeight > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION) ||
			(header.pixelDepth > D3D12_REQ_TEXTURE3D_U_V_OR_W_DIMENSION))
		{
			return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
		}
		break;

	default:
		return HRESULT_FROM_WIN32(ERROR_NOT_SUPPORTED);
	}

	// Create the texture
	{	
		uint32_t subresourceCount = max(header.numberOfMipmapLevels, 1u) * max(header.numberOfArrayElements, 1u);
		std::unique_ptr<D3D12_SUBRESOURCE_DATA[]> initData(new (std::nothrow) D3D12_SUBRESOURCE_DATA[subresourceCount]);
		if (!initData)
		{
			return E_OUTOFMEMORY;
		}

		auto dxgiFormat = static_cast<DXGI_FORMAT>(format);

		size_t skipMip = 0;
		size_t twidth = 0;
		size_t theight = 0;
		size_t tdepth = 0;
		hr = FillInitData(width, height, depth, numMips, arraySize, dxgiFormat, maxsize, ktxDataSize, ktxData + offset + sizeof(uint32_t), // TODO HACK
			twidth, theight, tdepth, skipMip, initData.get());

		if (SUCCEEDED(hr))
		{
			hr = CreateD3DResources(d3dDevice, resDim, twidth, theight, tdepth, numMips - skipMip, arraySize,
				dxgiFormat, forceSRGB,
				isCubeMap, texture, textureView);

			if (FAILED(hr) && !maxsize && (numMips > 1))
			{
				// Retry with a maxsize determined by feature level
				maxsize = (resDim == D3D12_RESOURCE_DIMENSION_TEXTURE3D)
					? 2048 /*D3D10_REQ_TEXTURE3D_U_V_OR_W_DIMENSION*/
					: 8192 /*D3D10_REQ_TEXTURE2D_U_OR_V_DIMENSION*/;

				hr = FillInitData(width, height, depth, numMips, arraySize, dxgiFormat, maxsize, ktxDataSize, ktxData + offset + sizeof(uint32_t), // TODO HACK
					twidth, theight, tdepth, skipMip, initData.get());

				if (SUCCEEDED(hr))
				{
					hr = CreateD3DResources(d3dDevice, resDim, twidth, theight, tdepth, numMips - skipMip, arraySize,
						dxgiFormat, forceSRGB,
						isCubeMap, texture, textureView);
				}
			}
		}

		if (SUCCEEDED(hr))
		{
			GpuResource destTexture(*texture, ResourceState::CopyDest);
			CommandContext::InitializeTexture(destTexture, subresourceCount, initData.get());
		}
	}

	return hr;
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