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
	float2 pos : POSITION;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD;
	float4 color : COLOR;
};


[[vk::binding(0, 0)]]
cbuffer VSConstants : register(b0)
{
	float2 scale;
	float2 translate;
}


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.uv = input.uv;
	output.color = input.color;
	output.pos = float4(input.pos * scale + translate, 0.0, 1.0);

#if !VK
	output.pos.y = -output.pos.y;
#endif

	return output;
}