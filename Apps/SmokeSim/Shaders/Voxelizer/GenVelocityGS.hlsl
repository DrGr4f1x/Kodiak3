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
	float3 vel : TEXCOORD;
};


struct GSOutput
{
	float4 pos : SV_Position;
	float3 vel : TEXCOORD;
	uint rtIndex : SV_RenderTargetArrayIndex;
};


[[vk::binding(0, 1)]]
cbuffer GSConstants
{
	int sliceIndex;
	float sliceZ;
	float2 projSpacePixDim;
};


struct VelocityVertex
{
	float2 pos;
	float3 vel;
};


void GetEdgePlaneIntersection(GSInput vtx0, GSInput vtx1, float sliceZ, inout VelocityVertex intersections[2], inout int index)
{
	float t = (sliceZ - vtx0.pos.z) / (vtx1.pos.z - vtx0.pos.z);
	if ((t < 0) || (t > 1))
		return;

	intersections[index].pos = lerp(vtx0.pos, vtx1.pos, t).xy;
	intersections[index].vel = lerp(vtx0.vel, vtx1.vel, t);
	++index;
}


[maxvertexcount(4)]
void main(triangle GSInput input[3], inout TriangleStream<GSOutput> triStream)
{
	GSOutput output = (GSOutput)0;
	output.rtIndex = sliceIndex;

	float minZ = min(min(input[0].pos.z, input[1].pos.z), input[2].pos.z);
	float maxZ = max(max(input[0].pos.z, input[1].pos.z), input[2].pos.z);

	if ((sliceZ < minZ) || (sliceZ > maxZ))
		return;

	VelocityVertex intersections[2];
	for (int i = 0; i < 2; ++i)
	{
		intersections[i].pos = 0.0;
		intersections[i].vel = 0.0;
	}

	int index = 0;
	if (index < 2)
		GetEdgePlaneIntersection(input[0], input[1], sliceZ, intersections, index);
	if (index < 2)
		GetEdgePlaneIntersection(input[1], input[2], sliceZ, intersections, index);
	if (index < 2)
		GetEdgePlaneIntersection(input[2], input[0], sliceZ, intersections, index);

	if (index < 2)
		return;

	const float2 sqrt2 = 1.4142857;
	float3 edge01 = input[1].pos.xyz - input[0].pos.xyz;
	float3 edge02 = input[2].pos.xyz - input[0].pos.xyz;
	float2 normal = sqrt2 * normalize(cross(edge01, edge02).xy);

	for (int j = 0; j < 2; ++j)
	{
		output.pos = float4(intersections[j].pos, 0.0, 1.0);
		output.vel = intersections[j].vel;
		triStream.Append(output);

		output.pos = float4((intersections[j].pos + (normal * projSpacePixDim)), 0.0, 1.0);
		output.vel = intersections[j].vel;
		triStream.Append(output);
	}
	triStream.RestartStrip();
}