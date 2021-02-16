//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

struct GSInput
{
	float4 pos : SV_Position;
	float3 uvw : TEXCOORD;
};


struct GSOutput
{
	float4 pos : SV_Position;
	float3 uvw : TEXCOORD;
	uint rtIndex : SV_RenderTargetArrayIndex;
};


[maxvertexcount(3)]
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> triStream)
{
	GSOutput output = (GSOutput)0;
	output.rtIndex = uint(input[0].uvw.z);

	for (uint i = 0; i < 3; i++)
	{
		output.pos = input[i].pos;
		output.uvw = input[i].uvw;
		triStream.Append(output);
	}
	triStream.RestartStrip();
}