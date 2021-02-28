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

#include "VertexTypes.h"


using namespace Kodiak;
using namespace std;


const VertexStreamDesc VertexPosition::Stream = { 0, sizeof(VertexPosition), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPosition::Layout =
{
	{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(VertexPosition, position), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionColor::Stream = { 0, sizeof(VertexPositionColor), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionColor::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float,		0, offsetof(VertexPositionColor, position), InputClassification::PerVertexData, 0 },
	{ "COLOR",		0, Format::R32G32B32A32_Float,	0, offsetof(VertexPositionColor, color), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionTexture::Stream = { 0, sizeof(VertexPositionTexture), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionTexture::Layout =
{
	{ "POSITION", 0, Format::R32G32B32_Float,	0, offsetof(VertexPositionTexture, position), InputClassification::PerVertexData, 0 },
	{ "TEXCOORD", 0, Format::R32G32_Float,		0, offsetof(VertexPositionTexture, texcoord), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionNormal::Stream = { 0, sizeof(VertexPositionNormal), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionNormal::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float, 0, offsetof(VertexPositionNormal, position), InputClassification::PerVertexData, 0 },
	{ "NORMAL",		0, Format::R32G32B32_Float, 0, offsetof(VertexPositionNormal, normal), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionColorTexture::Stream = { 0, sizeof(VertexPositionColorTexture), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionColorTexture::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float,		0, offsetof(VertexPositionColorTexture, position), InputClassification::PerVertexData, 0 },
	{ "COLOR",		0, Format::R32G32B32A32_Float,	0, offsetof(VertexPositionColorTexture, color), InputClassification::PerVertexData, 0 },
	{ "TEXCOORD",	0, Format::R32G32_Float,		0, offsetof(VertexPositionColorTexture, texcoord), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionNormalColor::Stream = { 0, sizeof(VertexPositionNormalColor), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionNormalColor::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float,		0, offsetof(VertexPositionNormalColor, position), InputClassification::PerVertexData, 0 },
	{ "NORMAL",		0, Format::R32G32B32_Float,		0, offsetof(VertexPositionNormalColor, normal), InputClassification::PerVertexData, 0 },
	{ "COLOR",		0, Format::R32G32B32A32_Float,	0, offsetof(VertexPositionNormalColor, color), InputClassification::PerVertexData, 0 },
};


const VertexStreamDesc VertexPositionNormalTexture::Stream = { 0, sizeof(VertexPositionNormalTexture), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionNormalTexture::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float, 0, offsetof(VertexPositionNormalTexture, position), InputClassification::PerVertexData, 0 },
	{ "NORMAL",		0, Format::R32G32B32_Float, 0, offsetof(VertexPositionNormalTexture, normal), InputClassification::PerVertexData, 0 },
	{ "TEXCOORD",	0, Format::R32G32_Float,	0, offsetof(VertexPositionNormalTexture, texcoord), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionNormalColorTexture::Stream = { 0, sizeof(VertexPositionNormalColorTexture), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionNormalColorTexture::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float,		0, offsetof(VertexPositionNormalColorTexture, position), InputClassification::PerVertexData, 0 },
	{ "NORMAL",		0, Format::R32G32B32_Float,		0, offsetof(VertexPositionNormalColorTexture, normal), InputClassification::PerVertexData, 0 },
	{ "COLOR",		0, Format::R32G32B32A32_Float,	0, offsetof(VertexPositionNormalColorTexture, color), InputClassification::PerVertexData, 0 },
	{ "TEXCOORD",	0, Format::R32G32_Float,		0, offsetof(VertexPositionNormalColorTexture, texcoord), InputClassification::PerVertexData, 0 }
};