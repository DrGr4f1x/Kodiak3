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
	[[vk::location(0)]] float3 pos : POSITION;
	[[vk::location(3)]] float3 color : COLOR;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 color : COLOR;
};


[[vk::binding(0, 0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4 lightPos;
}


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.pos = mul(projectionMatrix, mul(modelMatrix, float4(input.pos, 1.0)));
	output.color = input.color;

	return output;
}