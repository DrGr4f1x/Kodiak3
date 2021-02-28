//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Volume.hlsli"


float4 main(VSToPSRayCast input) : SV_Target
{
	float edge = EdgeTex.Sample(LinearClampSampler, float2(input.pos.xy * invRtDim)).r;

	float4 tex = RayCastTex.Sample(LinearClampSampler, float2(input.pos.xy * invRtDim));

	if (edge > 0.0 && tex.a > 0.0)
		return RayCast(input);

	return tex;
}