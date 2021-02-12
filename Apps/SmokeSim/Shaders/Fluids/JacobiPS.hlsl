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
    float pCenter = PressureTex.SampleLevel(PointClampSampler, input.uvw, 0).r;
    // Texture_tempvector contains the "divergence" computed by PS_DIVERGENCE
    float bC = PressureTex.SampleLevel(PointClampSampler, input.uvw, 0).r;

    float pL = PressureTex.SampleLevel(PointClampSampler, LEFTCELL, 0).r;
    float pR = PressureTex.SampleLevel(PointClampSampler, RIGHTCELL, 0).r;
    float pB = PressureTex.SampleLevel(PointClampSampler, BOTTOMCELL, 0).r;
    float pT = PressureTex.SampleLevel(PointClampSampler, TOPCELL, 0).r;
    float pD = PressureTex.SampleLevel(PointClampSampler, DOWNCELL, 0).r;
    float pU = PressureTex.SampleLevel(PointClampSampler, UPCELL, 0).r;

    if (IsBoundaryCell(LEFTCELL))  pL = pCenter;
    if (IsBoundaryCell(RIGHTCELL)) pR = pCenter;
    if (IsBoundaryCell(BOTTOMCELL))pB = pCenter;
    if (IsBoundaryCell(TOPCELL))   pT = pCenter;
    if (IsBoundaryCell(DOWNCELL))  pD = pCenter;
    if (IsBoundaryCell(UPCELL))    pU = pCenter;

    return(pL + pR + pB + pT + pU + pD - bC) / 6.0;
}