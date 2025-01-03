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


float4 main(VSToPSRayDataFront input) : SV_Target
{
	float sceneZ = SceneDepthTex.SampleLevel(LinearClampSampler, float2(input.pos.xy * invRtDim), 0).r;

	if (sceneZ < input.depth)
	{
		return OCCLUDED_PIXEL_RAYVALUE;
	}

	return float4(-input.posInGrid, input.depth);
}