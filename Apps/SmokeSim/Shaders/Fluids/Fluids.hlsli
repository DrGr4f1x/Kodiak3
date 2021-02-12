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


Texture3D VelocityTex0;
Texture3D VelocityTex1;
Texture3D ColorTex;
Texture3D ObstaclesTex;
Texture3D ObstVelocityTex;
Texture3D PressureTex;
Texture3D TempScalarTex;
Texture3D TempVectorTex;


SamplerState PointClampSampler;
SamplerState LinearSampler;


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