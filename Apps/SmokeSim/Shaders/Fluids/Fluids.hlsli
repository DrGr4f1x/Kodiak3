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


struct VSToGSData
{
	float4 pos :	SV_Position;
	float3 cell0 :	TEXCOORD0;
	float3 uvw :	TEXCOORD1;
	float2 LR :		TEXCOORD2;
	float2 BT :		TEXCOORD3;
	float2 DU :		TEXCOORD4;
};


struct GSToPSData
{
	float4 pos :	SV_POSITION;
	float3 cell0 :	TEXCOORD0;
	float3 uvw :	TEXCOORD1;
	float2 LR :		TEXCOORD2;
	float2 BT :		TEXCOORD3;
	float2 DU :		TEXCOORD4;
	uint RTIndex :	SV_RenderTArgetArrayIndex;
};


[[vk::binding(0, 0)]]
cbuffer SimConstants : register(b0)
{
	float3 texDim;
	float3 invTexDim;
	int texNumber;
	float4 obstVelocity;
	float modulate;
	float size;
	float3 center;
	float4 splatColor;
	float epsilon;
	float timestep;
	float forward;
	float3 halfVolumeDim;
	float3 boxLBDCorner;
	float3 boxRTUCorner;
};


[[vk::binding(0, 1)]] 
Texture3D VelocityTex0 : register(t0);
[[vk::binding(1, 1)]] 
Texture3D VelocityTex1 : register(t1);
[[vk::binding(2, 1)]] 
Texture3D ColorTex : register(t2);
[[vk::binding(3, 1)]] 
Texture3D ObstaclesTex : register(t3);
[[vk::binding(4, 1)]] 
Texture3D ObstVelocityTex : register(t4);
[[vk::binding(5, 1)]] 
Texture3D PressureTex : register(t5);
[[vk::binding(6, 1)]] 
Texture3D TempScalarTex : register(t6);
[[vk::binding(7, 1)]] 
Texture3D TempVectorTex : register(t7);


[[vk::binding(0, 2)]] 
SamplerState PointClampSampler : register(s0);
[[vk::binding(1, 2)]] 
SamplerState LinearSampler : register(s1);


#define LEFTCELL    float3 (input.LR.x, input.uvw.y, input.uvw.z)
#define RIGHTCELL   float3 (input.LR.y, input.uvw.y, input.uvw.z)
#define BOTTOMCELL  float3 (input.uvw.x, input.BT.x, input.uvw.z)
#define TOPCELL     float3 (input.uvw.x, input.BT.y, input.uvw.z)
#define DOWNCELL    float3 (input.uvw.x, input.uvw.y, input.DU.x)
#define UPCELL      float3 (input.uvw.x, input.uvw.y, input.DU.y)


float3 GetObstVelocity(float3 cellTexCoords)
{
    return ObstVelocityTex.SampleLevel(PointClampSampler, cellTexCoords, 0).xyz;
}


bool IsNonEmptyCell(float3 cellTexCoords)
{
    return (ObstaclesTex.SampleLevel(PointClampSampler, cellTexCoords, 0).r > 0.0);
}


bool IsBoundaryCell(float3 cellTexCoords)
{
    return (ObstaclesTex.SampleLevel(PointClampSampler, cellTexCoords, 0).r > 0.9);
}


float3 GetAdvectedPosTexCoords(GSToPSData input)
{
    float3 pos = input.cell0;

    pos -= timestep * forward * VelocityTex0.SampleLevel(PointClampSampler, input.uvw, 0).xyz;

    return float3(pos.x * invTexDim.x, pos.y * invTexDim.y, (pos.z + 0.5) * invTexDim.z);
}