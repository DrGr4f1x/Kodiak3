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
	float3 uvw : TEXCOORD;
	uint rtIndex : SV_RenderTargetArrayIndex;
};


[[vk::binding(0, 0)]]
Texture2D<uint2> StencilTex;


float4 main(PSInput input) : SV_TARGET
{
	if (StencilTex.Load(int3(input.uvw.x, input.uvw.y, 0)).g)
		return 0.5;
	return 0.0;
}