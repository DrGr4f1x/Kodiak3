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

struct PSOutput
{
	float4 obstacle : SV_TARGET0;
	float4 velocity : SV_TARGET1;
};


bool PointIsInsideBox(float3 p, float3 LBUcorner, float3 RTDcorner)
{
    return ((p.x > LBUcorner.x) && (p.x < RTDcorner.x)
        && (p.y > LBUcorner.y) && (p.y < RTDcorner.y)
        && (p.z > LBUcorner.z) && (p.z < RTDcorner.z));
}


PSOutput main(GSToPSData input)
{
    PSOutput voxel = (PSOutput)0;

    float3 innerBoxLBDCorner = boxLBDCorner + 1.0;
    float3 innerBoxRTUCorner = boxRTUCorner - 1.0;

    // cells completely inside box = 1.0
    if (PointIsInsideBox(input.cell0, innerBoxLBDCorner, innerBoxRTUCorner))
    {
        voxel.obstacle = 0.5;
        voxel.velocity = 0.0;
        return voxel;
    }

    // cells in box boundary = 0.5
    if (PointIsInsideBox(input.cell0, boxLBDCorner, boxRTUCorner))
    {
        voxel.obstacle = 1.0;
        voxel.velocity = float4(obstVelocity.xyz, 1.0);
        return voxel;
    }

    return voxel;
}