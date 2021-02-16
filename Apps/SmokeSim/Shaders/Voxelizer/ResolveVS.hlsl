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
	float3 uvw : TEXCOORD;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 uvw : TEXCOORD;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.pos = float4(input.pos, 1.0);
	output.uvw = input.uvw;

	return output;
}