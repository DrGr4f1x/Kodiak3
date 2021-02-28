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


VSToPSEdgeDetect main(VSInput input)
{
	VSToPSEdgeDetect output = (VSToPSEdgeDetect)0;

	output.pos = float4(input.pos, 1.0);
	
	float2 center = float2((input.pos.x + 1.0) / 2.0, 1.0 - (input.pos.y + 1.0) / 2.0);

	output.uv00 = center + float2(-invRtDim.x, -invRtDim.y);
	output.uv01 = center + float2(-invRtDim.x, 0.0);
	output.uv02 = center + float2(-invRtDim.x,  invRtDim.y);

	output.uv10 = center + float2(0.0, -invRtDim.y);
	output.uv12 = center + float2(0.0,  invRtDim.y);

	output.uv20 = center + float2(invRtDim.x, -invRtDim.y);
	output.uv21 = center + float2(invRtDim.x, 0.0);
	output.uv22 = center + float2(invRtDim.x,  invRtDim.y);

	return output;
}