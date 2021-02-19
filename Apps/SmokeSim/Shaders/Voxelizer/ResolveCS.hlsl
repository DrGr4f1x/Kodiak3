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
Texture2D<uint2>		StencilTex;
[[vk::binding(1, 0)]]
RWTexture3D<float>		VolumeTex;

[[vk::binding(2, 0)]]
cbuffer CSConstants
{
	uint width;
	uint height;
	uint depth;
	uint rows;
	uint cols;
};


[numthreads(8, 8, 1)]
void main( uint3 GTid : SV_GroupThreadID, uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID )
{
	uint z = DTid.z;

	uint row = z / cols;
	uint col = z % cols;
	int x = int(float(col) * float(width) + 0.5) + Gid.x * 8 + GTid.x;
	int y = int(float(row) * float(height) + 0.5) + Gid.y * 8 + GTid.y;

	float res = 0.0;
	if (StencilTex.Load(int3(x, y, 1)).g)
		res = 0.5;

	VolumeTex[DTid] = res;
}