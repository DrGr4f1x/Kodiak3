//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

struct PSInput
{
	float4 pos : SV_Position;
	float3 vel : TEXCOORD;
	uint rtIndex : SV_RenderTargetArrayIndex;
};


struct PSOutput
{
	float4 velocity : SV_Target0;
	float obstacle : SV_Target1;
};


PSOutput main(PSInput input)
{
	PSOutput output = (PSOutput)0;

	output.velocity = float4(input.vel, 1.0);
	output.obstacle = 1.0;

	return output;
}