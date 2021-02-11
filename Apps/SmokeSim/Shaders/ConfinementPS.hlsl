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
    // Texture_tempvector contains the vorticity computed by PS_VORTICITY
    float4 omega = TempVectorTex.SampleLevel(PointClampSampler, input.uvw, 0);

    // Potential optimization: don't find length multiple times - do once for the entire texture
    float omegaL = length(TempVectorTex.SampleLevel(PointClampSampler, LEFTCELL, 0));
    float omegaR = length(TempVectorTex.SampleLevel(PointClampSampler, RIGHTCELL, 0));
    float omegaB = length(TempVectorTex.SampleLevel(PointClampSampler, BOTTOMCELL, 0));
    float omegaT = length(TempVectorTex.SampleLevel(PointClampSampler, TOPCELL, 0));
    float omegaD = length(TempVectorTex.SampleLevel(PointClampSampler, DOWNCELL, 0));
    float omegaU = length(TempVectorTex.SampleLevel(PointClampSampler, UPCELL, 0));

    float3 eta = 0.5 * float3(omegaR - omegaL,
                              omegaT - omegaB,
                              omegaU - omegaD);

    eta = normalize(eta + float3(0.001, 0.001, 0.001));

    float4 force;
    force.w = 0.0;
    force.xyz = timestep * epsilon * float3(eta.y * omega.z - eta.z * omega.y,
                                            eta.z * omega.x - eta.x * omega.z,
                                            eta.x * omega.y - eta.y * omega.x);

    // Note: the result is added to the current velocity at each cell using "additive blending"
    return force;
}