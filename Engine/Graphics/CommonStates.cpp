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

#include "CommonStates.h"

#include "PipelineState.h"
#include "SamplerState.h"


using namespace Kodiak;
using namespace std;


const BlendStateDesc& CommonStates::BlendNoColorWrite()
{
	static BlendStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.alphaToCoverageEnable = false;
		desc.independentBlendEnable = false;
		for (auto& renderTargetBlend : desc.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = false;
			renderTargetBlend.srcBlend = Blend::SrcAlpha;
			renderTargetBlend.dstBlend = Blend::InvSrcAlpha;
			renderTargetBlend.blendOp = BlendOp::Add;
			renderTargetBlend.srcBlendAlpha = Blend::One;
			renderTargetBlend.dstBlendAlpha = Blend::InvSrcAlpha;
			renderTargetBlend.blendOpAlpha = BlendOp::Add;
			renderTargetBlend.writeMask = ColorWrite::None;
		}
		initialized = true;
	}

	return desc;
}


const BlendStateDesc& CommonStates::BlendDisable()
{
	static BlendStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.alphaToCoverageEnable = false;
		desc.independentBlendEnable = false;
		for (auto& renderTargetBlend : desc.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = false;
			renderTargetBlend.srcBlend = Blend::SrcAlpha;
			renderTargetBlend.dstBlend = Blend::InvSrcAlpha;
			renderTargetBlend.blendOp = BlendOp::Add;
			renderTargetBlend.srcBlendAlpha = Blend::One;
			renderTargetBlend.dstBlendAlpha = Blend::InvSrcAlpha;
			renderTargetBlend.blendOpAlpha = BlendOp::Add;
			renderTargetBlend.writeMask = ColorWrite::All;
		}
		initialized = true;
	}

	return desc;
}


const BlendStateDesc& CommonStates::BlendPreMultiplied()
{
	static BlendStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.alphaToCoverageEnable = false;
		desc.independentBlendEnable = false;
		for (auto& renderTargetBlend : desc.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = true;
			renderTargetBlend.srcBlend = Blend::One;
			renderTargetBlend.dstBlend = Blend::InvSrcAlpha;
			renderTargetBlend.blendOp = BlendOp::Add;
			renderTargetBlend.srcBlendAlpha = Blend::One;
			renderTargetBlend.dstBlendAlpha = Blend::InvSrcAlpha;
			renderTargetBlend.blendOpAlpha = BlendOp::Add;
			renderTargetBlend.writeMask = ColorWrite::All;
		}
		initialized = true;
	}

	return desc;
}


const BlendStateDesc& CommonStates::BlendTraditional()
{
	static BlendStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.alphaToCoverageEnable = false;
		desc.independentBlendEnable = false;
		for (auto& renderTargetBlend : desc.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = true;
			renderTargetBlend.srcBlend = Blend::SrcAlpha;
			renderTargetBlend.dstBlend = Blend::InvSrcAlpha;
			renderTargetBlend.blendOp = BlendOp::Add;
			renderTargetBlend.srcBlendAlpha = Blend::One;
			renderTargetBlend.dstBlendAlpha = Blend::InvSrcAlpha;
			renderTargetBlend.blendOpAlpha = BlendOp::Add;
			renderTargetBlend.writeMask = ColorWrite::All;
		}
		initialized = true;
	}

	return desc;
}


const BlendStateDesc& CommonStates::BlendAdditive()
{
	static BlendStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.alphaToCoverageEnable = false;
		desc.independentBlendEnable = false;
		for (auto& renderTargetBlend : desc.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = true;
			renderTargetBlend.srcBlend = Blend::One;
			renderTargetBlend.dstBlend = Blend::One;
			renderTargetBlend.blendOp = BlendOp::Add;
			renderTargetBlend.srcBlendAlpha = Blend::One;
			renderTargetBlend.dstBlendAlpha = Blend::InvSrcAlpha;
			renderTargetBlend.blendOpAlpha = BlendOp::Add;
			renderTargetBlend.writeMask = ColorWrite::All;
		}
		initialized = true;
	}

	return desc;
}


const BlendStateDesc& CommonStates::BlendTraditionalAdditive()
{
	static BlendStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.alphaToCoverageEnable = false;
		desc.independentBlendEnable = false;
		for (auto& renderTargetBlend : desc.renderTargetBlend)
		{
			renderTargetBlend.blendEnable = true;
			renderTargetBlend.srcBlend = Blend::SrcAlpha;
			renderTargetBlend.dstBlend = Blend::One;
			renderTargetBlend.blendOp = BlendOp::Add;
			renderTargetBlend.srcBlendAlpha = Blend::One;
			renderTargetBlend.dstBlendAlpha = Blend::InvSrcAlpha;
			renderTargetBlend.blendOpAlpha = BlendOp::Add;
			renderTargetBlend.writeMask = ColorWrite::All;
		}
		initialized = true;
	}

	return desc;
}


const RasterizerStateDesc& CommonStates::RasterizerDefault()
{
	static RasterizerStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.fillMode = FillMode::Solid;
		desc.cullMode = CullMode::Back;
		desc.frontCounterClockwise = true;
		desc.depthBias = Defaults::DepthBias;
		desc.depthBiasClamp = Defaults::DepthBiasClamp;
		desc.slopeScaledDepthBias = Defaults::SlopeScaledDepthBias;
		desc.depthClipEnable = true;
		desc.multisampleEnable = false;
		desc.antialiasedLineEnable = false;
		desc.forcedSampleCount = 0;
		desc.conservativeRasterizationEnable = false;

		initialized = true;
	}

	return desc;
}


const RasterizerStateDesc& CommonStates::RasterizerDefaultCW()
{
	static RasterizerStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.fillMode = FillMode::Solid;
		desc.cullMode = CullMode::Back;
		desc.frontCounterClockwise = false;
		desc.depthBias = Defaults::DepthBias;
		desc.depthBiasClamp = Defaults::DepthBiasClamp;
		desc.slopeScaledDepthBias = Defaults::SlopeScaledDepthBias;
		desc.depthClipEnable = true;
		desc.multisampleEnable = false;
		desc.antialiasedLineEnable = false;
		desc.forcedSampleCount = 0;
		desc.conservativeRasterizationEnable = false;

		initialized = true;
	}

	return desc;
}


const RasterizerStateDesc& CommonStates::RasterizerTwoSided()
{
	static RasterizerStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.fillMode = FillMode::Solid;
		desc.cullMode = CullMode::None;
		desc.frontCounterClockwise = true;
		desc.depthBias = Defaults::DepthBias;
		desc.depthBiasClamp = Defaults::DepthBiasClamp;
		desc.slopeScaledDepthBias = Defaults::SlopeScaledDepthBias;
		desc.depthClipEnable = true;
		desc.multisampleEnable = false;
		desc.antialiasedLineEnable = false;
		desc.forcedSampleCount = 0;
		desc.conservativeRasterizationEnable = false;

		initialized = true;
	}

	return desc;
}


const RasterizerStateDesc& CommonStates::RasterizerShadow()
{
	static RasterizerStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.fillMode = FillMode::Solid;
		desc.cullMode = CullMode::Back;
		desc.frontCounterClockwise = true;
		desc.depthBias = -100;
		desc.depthBiasClamp = Defaults::DepthBiasClamp;
		desc.slopeScaledDepthBias = -1.5f;
		desc.depthClipEnable = true;
		desc.multisampleEnable = false;
		desc.antialiasedLineEnable = false;
		desc.forcedSampleCount = 0;
		desc.conservativeRasterizationEnable = false;

		initialized = true;
	}

	return desc;
}


const RasterizerStateDesc& CommonStates::RasterizerShadowCW()
{
	static RasterizerStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.fillMode = FillMode::Solid;
		desc.cullMode = CullMode::Back;
		desc.frontCounterClockwise = false;
		desc.depthBias = -100;
		desc.depthBiasClamp = Defaults::DepthBiasClamp;
		desc.slopeScaledDepthBias = -1.5f;
		desc.depthClipEnable = true;
		desc.multisampleEnable = false;
		desc.antialiasedLineEnable = false;
		desc.forcedSampleCount = 0;
		desc.conservativeRasterizationEnable = false;

		initialized = true;
	}

	return desc;
}


const RasterizerStateDesc& CommonStates::RasterizerWireframe()
{
	static RasterizerStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.fillMode = FillMode::Wireframe;
		desc.cullMode = CullMode::None;
		desc.frontCounterClockwise = true;
		desc.depthBias = Defaults::DepthBias;
		desc.depthBiasClamp = Defaults::DepthBiasClamp;
		desc.slopeScaledDepthBias = Defaults::SlopeScaledDepthBias;
		desc.depthClipEnable = true;
		desc.multisampleEnable = false;
		desc.antialiasedLineEnable = false;
		desc.forcedSampleCount = 0;
		desc.conservativeRasterizationEnable = false;

		initialized = true;
	}

	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthStateDisabled()
{
	static DepthStencilStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.depthEnable = false;
		desc.depthWriteMask = DepthWrite::Zero;
		desc.depthFunc = ComparisonFunc::Always;
		desc.stencilEnable = false;
		desc.stencilReadMask = Defaults::StencilReadMask;
		desc.stencilWriteMask = Defaults::StencilWriteMask;
		desc.frontFace.stencilFunc = ComparisonFunc::Always;
		desc.frontFace.stencilPassOp = StencilOp::Keep;
		desc.frontFace.stencilFailOp = StencilOp::Keep;
		desc.frontFace.stencilDepthFailOp = StencilOp::Keep;
		desc.backFace = desc.frontFace;

		initialized = true;
	}

	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthStateReadWrite()
{
	static DepthStencilStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.depthEnable = true;
		desc.depthWriteMask = DepthWrite::All;
		desc.depthFunc = ComparisonFunc::GreaterEqual;
		desc.stencilEnable = false;
		desc.stencilReadMask = Defaults::StencilReadMask;
		desc.stencilWriteMask = Defaults::StencilWriteMask;
		desc.frontFace.stencilFunc = ComparisonFunc::Always;
		desc.frontFace.stencilPassOp = StencilOp::Keep;
		desc.frontFace.stencilFailOp = StencilOp::Keep;
		desc.frontFace.stencilDepthFailOp = StencilOp::Keep;
		desc.backFace = desc.frontFace;

		initialized = true;
	}

	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthStateReadWriteReversed()
{
	static DepthStencilStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.depthEnable = true;
		desc.depthWriteMask = DepthWrite::All;
		desc.depthFunc = ComparisonFunc::LessEqual;
		desc.stencilEnable = false;
		desc.stencilReadMask = Defaults::StencilReadMask;
		desc.stencilWriteMask = Defaults::StencilWriteMask;
		desc.frontFace.stencilFunc = ComparisonFunc::Always;
		desc.frontFace.stencilPassOp = StencilOp::Keep;
		desc.frontFace.stencilFailOp = StencilOp::Keep;
		desc.frontFace.stencilDepthFailOp = StencilOp::Keep;
		desc.backFace = desc.frontFace;

		initialized = true;
	}

	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthStateReadOnly()
{
	static DepthStencilStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.depthEnable = true;
		desc.depthWriteMask = DepthWrite::Zero;
		desc.depthFunc = ComparisonFunc::GreaterEqual;
		desc.stencilEnable = false;
		desc.stencilReadMask = Defaults::StencilReadMask;
		desc.stencilWriteMask = Defaults::StencilWriteMask;
		desc.frontFace.stencilFunc = ComparisonFunc::Always;
		desc.frontFace.stencilPassOp = StencilOp::Keep;
		desc.frontFace.stencilFailOp = StencilOp::Keep;
		desc.frontFace.stencilDepthFailOp = StencilOp::Keep;
		desc.backFace = desc.frontFace;

		initialized = true;
	}

	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthStateReadOnlyReversed()
{
	static DepthStencilStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.depthEnable = true;
		desc.depthWriteMask = DepthWrite::Zero;
		desc.depthFunc = ComparisonFunc::Less;
		desc.stencilEnable = false;
		desc.stencilReadMask = Defaults::StencilReadMask;
		desc.stencilWriteMask = Defaults::StencilWriteMask;
		desc.frontFace.stencilFunc = ComparisonFunc::Always;
		desc.frontFace.stencilPassOp = StencilOp::Keep;
		desc.frontFace.stencilFailOp = StencilOp::Keep;
		desc.frontFace.stencilDepthFailOp = StencilOp::Keep;
		desc.backFace = desc.frontFace;

		initialized = true;
	}

	return desc;
}


const DepthStencilStateDesc& CommonStates::DepthStateTestEqual()
{
	static DepthStencilStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.depthEnable = true;
		desc.depthWriteMask = DepthWrite::Zero;
		desc.depthFunc = ComparisonFunc::Equal;
		desc.stencilEnable = false;
		desc.stencilReadMask = Defaults::StencilReadMask;
		desc.stencilWriteMask = Defaults::StencilWriteMask;
		desc.frontFace.stencilFunc = ComparisonFunc::Always;
		desc.frontFace.stencilPassOp = StencilOp::Keep;
		desc.frontFace.stencilFailOp = StencilOp::Keep;
		desc.frontFace.stencilDepthFailOp = StencilOp::Keep;
		desc.backFace = desc.frontFace;

		initialized = true;
	}

	return desc;
}


const SamplerStateDesc& CommonStates::SamplerLinearWrap()
{
	static SamplerStateDesc desc{ TextureFilter::MinMagMipLinear };
	return desc;
}


const SamplerStateDesc& CommonStates::SamplerAnisoWrap()
{
	static SamplerStateDesc desc{};
	static bool initialized = false;

	if (!initialized)
	{
		desc.maxAnisotropy = 8;
		initialized = true;
	}

	return desc;
}


const SamplerStateDesc& CommonStates::SamplerShadow()
{
	static SamplerStateDesc desc{ TextureFilter::ComparisonMinMagLinearMipPoint, TextureAddress::Clamp };
	static bool initialized = false;

	if (!initialized)
	{
		desc.comparisonFunc = ComparisonFunc::GreaterEqual;
		initialized = true;
	}

	return desc;
}


const SamplerStateDesc& CommonStates::SamplerLinearClamp()
{
	static SamplerStateDesc desc{ TextureFilter::MinMagMipLinear, TextureAddress::Clamp };
	return desc;
}


const SamplerStateDesc& CommonStates::SamplerVolumeWrap()
{
	static SamplerStateDesc desc{ TextureFilter::MinMagMipPoint };
	return desc;
}


const SamplerStateDesc& CommonStates::SamplerPointClamp()
{
	static SamplerStateDesc desc{ TextureFilter::MinMagMipPoint, TextureAddress::Clamp };
	return desc;
}


const SamplerStateDesc& CommonStates::SamplerPointBorder()
{
	static SamplerStateDesc desc{ TextureFilter::MinMagMipPoint, TextureAddress::Border };
	static bool initialized = false;

	if (!initialized)
	{
		desc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
		initialized = true;
	}

	return desc;
}


const SamplerStateDesc& CommonStates::SamplerLinearBorder()
{
	static SamplerStateDesc desc{ TextureFilter::MinMagMipLinear, TextureAddress::Border };
	static bool initialized = false;

	if (!initialized)
	{
		desc.SetBorderColor(Color(0.0f, 0.0f, 0.0f, 0.0f));
		initialized = true;
	}

	return desc;
}


const SamplerStateDesc& CommonStates::SamplerLinearMirror()
{
	static SamplerStateDesc desc{ TextureFilter::MinMagMipLinear, TextureAddress::Mirror };
	return desc;
}