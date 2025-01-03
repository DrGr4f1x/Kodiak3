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
	float3 position : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
};

struct VSOutput
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 eyePos : TEXCOORD0;
	float3 lightDir : TEXCOORD1;
	float2 texcoord : TEXCOORD2;
};

[[vk::binding(0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float gradientPos;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.normal = input.normal;
	output.color = input.color;
	output.texcoord = float2(gradientPos, 0.0f);

	float4x4 modelToProjection = mul(projectionMatrix, modelMatrix);
	output.position = mul(modelToProjection, float4(input.position, 1.0f));

	output.eyePos = mul(modelMatrix, float4(input.position, 1.0f)).xyz;

	float3 lightPos = float3(0.0f, 0.0f, -5.0f);
	output.lightDir = normalize(lightPos - input.position);

	return output;
}