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
    float4 fieldL = VelocityTex1.SampleLevel(PointClampSampler, LEFTCELL, 0);
    float4 fieldR = VelocityTex1.SampleLevel(PointClampSampler, RIGHTCELL, 0);
    float4 fieldB = VelocityTex1.SampleLevel(PointClampSampler, BOTTOMCELL, 0);
    float4 fieldT = VelocityTex1.SampleLevel(PointClampSampler, TOPCELL, 0);
    float4 fieldD = VelocityTex1.SampleLevel(PointClampSampler, DOWNCELL, 0);
    float4 fieldU = VelocityTex1.SampleLevel(PointClampSampler, UPCELL, 0);

    if (IsBoundaryCell(LEFTCELL))  fieldL = GetObstVelocity(LEFTCELL);
    if (IsBoundaryCell(RIGHTCELL)) fieldR = GetObstVelocity(RIGHTCELL);
    if (IsBoundaryCell(BOTTOMCELL))fieldB = GetObstVelocity(BOTTOMCELL);
    if (IsBoundaryCell(TOPCELL))   fieldT = GetObstVelocity(TOPCELL);
    if (IsBoundaryCell(DOWNCELL))  fieldD = GetObstVelocity(DOWNCELL);
    if (IsBoundaryCell(UPCELL))    fieldU = GetObstVelocity(UPCELL);

    float divergence = 0.5 *
        ((fieldR.x - fieldL.x) + (fieldT.y - fieldB.y) + (fieldU.z - fieldD.z));

    return divergence;
}