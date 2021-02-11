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


VSToGSData main(VSInput input)
{
	VSToGSData output = (VSToGSData)0;

	output.pos = float4(input.pos, 1.0);
	output.cell0 = input.uvw;
	output.uvw = input.uvw * invTexDim;

	float x = output.uvw.x;
	float y = output.uvw.y;
	float z = output.uvw.z;

	output.LR = float2(x - invTexDim.x, x + invTexDim.x);
	output.BT = float2(y - invTexDim.y, y + invTexDim.y);
	output.DU = float2(z - invTexDim.z, z + invTexDim.z);

	return output;
}