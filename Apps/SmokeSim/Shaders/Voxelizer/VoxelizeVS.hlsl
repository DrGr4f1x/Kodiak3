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
};


struct Matrices
{
	float4x4 modelViewProjectionMatrix;
};
#if VK
[[vk::push_constant]]
Matrices matrices;
#else
cbuffer VSConstants
{
	Matrices matrices;
};
#endif


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.pos = mul(matrices.modelViewProjectionMatrix, float4(input.pos, 1.0));

	return output;
}