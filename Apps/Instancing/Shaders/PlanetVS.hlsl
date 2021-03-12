//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

[[vk::binding(0, 0)]]
cbuffer VSConstants
{
	float4x4 projectionMatrix;
	float4x4 modelViewMatrix;
	float4 lightPos;
};


struct VSInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float4 color : COLOR;
	float2 uv : TEXCOORD;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float2 uv : TEXCOORD0;
	float3 viewVec : TEXCOORD1;
	float3 lightVec : TEXCOORD2;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	float4 pos = mul(modelViewMatrix, float4(input.pos, 1.0));

	output.pos = mul(projectionMatrix, pos);
	output.color = input.color.rgb;
	output.uv = float2(10.0, 6.0) * input.uv;

	output.normal = mul((float3x3)modelViewMatrix, input.normal);
	
	float3 lpos = mul((float3x3)modelViewMatrix, lightPos.xyz);

	output.lightVec = lpos.xyz - pos.xyz;
	output.viewVec = -pos.xyz;

	return output;
}