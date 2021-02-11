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


float4 main(GSToPSData input) : SV_TARGET
{
    if (IsBoundaryCell(input.uvw))
        return GetObstVelocity(input.uvw);

    float pCenter = PressureTex.SampleLevel(PointClampSampler, input.uvw, 0).r;
    float pL = PressureTex.SampleLevel(PointClampSampler, LEFTCELL, 0).r;
    float pR = PressureTex.SampleLevel(PointClampSampler, RIGHTCELL, 0).r;
    float pB = PressureTex.SampleLevel(PointClampSampler, BOTTOMCELL, 0).r;
    float pT = PressureTex.SampleLevel(PointClampSampler, TOPCELL, 0).r;
    float pD = PressureTex.SampleLevel(PointClampSampler, DOWNCELL, 0).r;
    float pU = PressureTex.SampleLevel(PointClampSampler, UPCELL, 0).r;

    float3 obstV = float3(0.0, 0.0, 0.0);
    float3 vMask = float3(1.0, 1.0, 1.0);
    float3 vLeft = GetObstVelocity(LEFTCELL);
    float3 vRight = GetObstVelocity(RIGHTCELL);
    float3 vBottom = GetObstVelocity(BOTTOMCELL);
    float3 vTop = GetObstVelocity(TOPCELL);
    float3 vDown = GetObstVelocity(DOWNCELL);
    float3 vUp = GetObstVelocity(UPCELL);

    if (IsBoundaryCell(LEFTCELL)) { pL = pCenter; obstV.x = vLeft.x; vMask.x = 0; }
    if (IsBoundaryCell(RIGHTCELL)) { pR = pCenter; obstV.x = vRight.x; vMask.x = 0; }
    if (IsBoundaryCell(BOTTOMCELL)) { pB = pCenter; obstV.y = vBottom.y; vMask.y = 0; }
    if (IsBoundaryCell(TOPCELL)) { pT = pCenter; obstV.y = vTop.y; vMask.y = 0; }
    if (IsBoundaryCell(DOWNCELL)) { pD = pCenter; obstV.z = vDown.z; vMask.z = 0; }
    if (IsBoundaryCell(UPCELL)) { pU = pCenter; obstV.z = vUp.z; vMask.z = 0; }

    float3 v = (VelocityTex1.SampleLevel(PointClampSampler, input.uvw, 0).xyz -
                 (0.5 * modulate * float3(pR - pL, pT - pB, pU - pD)));

    float4 velocity = float4(0.0, 0.0, 0.0, 0.0);
    velocity.xyz = (vMask * v) + obstV;

    return velocity;
}