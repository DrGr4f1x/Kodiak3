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


VSToPSRayDataBack main(VSInput input)
{
	VSToPSRayDataBack output = (VSToPSRayDataBack)0;

	output.pos = mul(worldProjectionMatrix, float4(input.pos, 1.0));
	output.depth = output.pos.w;

	return output;
}