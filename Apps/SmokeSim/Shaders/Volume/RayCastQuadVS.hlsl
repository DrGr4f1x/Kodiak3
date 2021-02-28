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


VSToPSRayCast main(VSInput input)
{
	VSToPSRayCast output = (VSToPSRayCast)0;

	output.pos = float4(input.pos, 1.0);
	output.posInGrid = mul(projectionWorldMatrix, float4(input.pos.xy * zNear, 0.0, zNear)).xyz;

	return output;
}