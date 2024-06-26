//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

struct VSInput
{
	float3 pos : POSITION;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 vel : TEXCOORD;
};


[[vk::binding(0, 0)]]
cbuffer VSConstants
{
	float4x4 modelViewProjectionMatrix;
	float4x4 prevModelViewProjectionMatrix;
	float3 gridDim;
	float deltaT;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	float4 gridPos = mul(modelViewProjectionMatrix, float4(input.pos, 1.0));
	float4 prevGridPos = mul(prevModelViewProjectionMatrix, float4(input.pos, 1.0));

	output.pos = gridPos;
	output.vel = (gridPos.xyz - prevGridPos.xyz) * 0.5 * gridDim * deltaT;

	return output;
}