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
	{ "POSITION", 0, Format::R32G32B32_Float, 0, offsetof(VertexPosition, x), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionColor::Stream = { 0, sizeof(VertexPositionColor), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionColor::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float,		0, offsetof(VertexPositionColor, x), InputClassification::PerVertexData, 0 },
	{ "COLOR",		0, Format::R32G32B32A32_Float,	0, offsetof(VertexPositionColor, r), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionTexture::Stream = { 0, sizeof(VertexPositionTexture), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionTexture::Layout =
{
	{ "POSITION", 0, Format::R32G32B32_Float,	0, offsetof(VertexPositionTexture, x), InputClassification::PerVertexData, 0 },
	{ "TEXCOORD", 0, Format::R32G32_Float,		0, offsetof(VertexPositionTexture, u), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionNormal::Stream = { 0, sizeof(VertexPositionNormal), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionNormal::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float, 0, offsetof(VertexPositionNormal, x), InputClassification::PerVertexData, 0 },
	{ "NORMAL",		0, Format::R32G32B32_Float, 0, offsetof(VertexPositionNormal, nx), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionColorTexture::Stream = { 0, sizeof(VertexPositionColorTexture), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionColorTexture::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float,		0, offsetof(VertexPositionColorTexture, x), InputClassification::PerVertexData, 0 },
	{ "COLOR",		0, Format::R32G32B32A32_Float,	0, offsetof(VertexPositionColorTexture, r), InputClassification::PerVertexData, 0 },
	{ "TEXCOORD",	0, Format::R32G32_Float,		0, offsetof(VertexPositionColorTexture, u), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionNormalColor::Stream = { 0, sizeof(VertexPositionNormalColor), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionNormalColor::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float,		0, offsetof(VertexPositionNormalColor, x), InputClassification::PerVertexData, 0 },
	{ "NORMAL",		0, Format::R32G32B32_Float,		0, offsetof(VertexPositionNormalColor, nx), InputClassification::PerVertexData, 0 },
	{ "COLOR",		0, Format::R32G32B32A32_Float,	0, offsetof(VertexPositionNormalColor, r), InputClassification::PerVertexData, 0 },
};


const VertexStreamDesc VertexPositionNormalTexture::Stream = { 0, sizeof(VertexPositionNormalTexture), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionNormalTexture::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float, 0, offsetof(VertexPositionNormalTexture, x), InputClassification::PerVertexData, 0 },
	{ "NORMAL",		0, Format::R32G32B32_Float, 0, offsetof(VertexPositionNormalTexture, nx), InputClassification::PerVertexData, 0 },
	{ "TEXCOORD",	0, Format::R32G32_Float,	0, offsetof(VertexPositionNormalTexture, u), InputClassification::PerVertexData, 0 }
};


const VertexStreamDesc VertexPositionNormalColorTexture::Stream = { 0, sizeof(VertexPositionNormalColorTexture), InputClassification::PerVertexData };
const vector<VertexElementDesc> VertexPositionNormalColorTexture::Layout =
{
	{ "POSITION",	0, Format::R32G32B32_Float,		0, offsetof(VertexPositionNormalColorTexture, x), InputClassification::PerVertexData, 0 },
	{ "NORMAL",		0, Format::R32G32B32_Float,		0, offsetof(VertexPositionNormalColorTexture, nx), InputClassification::PerVertexData, 0 },
	{ "COLOR",		0, Format::R32G32B32A32_Float,	0, offsetof(VertexPositionNormalColorTexture, r), InputClassification::PerVertexData, 0 },
	{ "TEXCOORD",	0, Format::R32G32_Float,		0, offsetof(VertexPositionNormalColorTexture, u), InputClassification::PerVertexData, 0 }
};