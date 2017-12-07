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

// Forward declarations
struct BlendStateDesc;
struct DepthStencilStateDesc;
struct RasterizerStateDesc;
struct SamplerStateDesc;


class CommonStates
{
public:
	// Blend states
	static const BlendStateDesc& BlendNoColorWrite();			// XXX
	static const BlendStateDesc& BlendDisable();				// 1, 0
	static const BlendStateDesc& BlendPreMultiplied();			// 1, 1-SrcA
	static const BlendStateDesc& BlendTraditional();			// SrcA, 1-SrcA
	static const BlendStateDesc& BlendAdditive();				// 1, 1
	static const BlendStateDesc& BlendTraditionalAdditive();	// SrcA, 1

	// Rasterizer states
	static const RasterizerStateDesc& RasterizerDefault();
	static const RasterizerStateDesc& RasterizerDefaultCW();
	static const RasterizerStateDesc& RasterizerTwoSided();
	static const RasterizerStateDesc& RasterizerShadow();
	static const RasterizerStateDesc& RasterizerShadowCW();
	static const RasterizerStateDesc& RasterizerWireframe();

	// Depth stencil states
	static const DepthStencilStateDesc& DepthStateDisabled();
	static const DepthStencilStateDesc& DepthStateReadWrite();
	static const DepthStencilStateDesc& DepthStateReadWriteReversed();
	static const DepthStencilStateDesc& DepthStateReadOnly();
	static const DepthStencilStateDesc& DepthStateReadOnlyReversed();
	static const DepthStencilStateDesc& DepthStateTestEqual();

	// Sampler states
	static const SamplerStateDesc& SamplerLinearWrap();
	static const SamplerStateDesc& SamplerAnisoWrap();
	static const SamplerStateDesc& SamplerShadow();
	static const SamplerStateDesc& SamplerLinearClamp();
	static const SamplerStateDesc& SamplerVolumeWrap();
	static const SamplerStateDesc& SamplerPointClamp();
	static const SamplerStateDesc& SamplerPointBorder();
	static const SamplerStateDesc& SamplerLinearBorder();
};

} // namespace Kodiak