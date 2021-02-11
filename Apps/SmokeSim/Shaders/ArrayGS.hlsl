//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Fluids.hlsli"


[maxvertexcount(3)]
void main(triangle VSToGSData input[3], inout TriangleStream<GSToPSData> triStream)
{
	GSToPSData output = (GSToPSData)0;
	output.RTIndex = input[0].cell0.z;
	for (uint i = 0; i < 3; i++)
	{
		output.pos = input[i].pos;
		output.cell0 = input[i].cell0;
		output.uvw = input[i].uvw;
		output.LR = input[i].LR;
		output.BT = input[i].BT;
		output.DU = input[i].DU;
		triStream.Append(output);
	}
	triStream.RestartStrip();
}