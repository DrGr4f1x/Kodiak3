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
    float4 L = VelocityTex1.SampleLevel(PointClampSampler, LEFTCELL, 0);
    float4 R = VelocityTex1.SampleLevel(PointClampSampler, RIGHTCELL, 0);
    float4 B = VelocityTex1.SampleLevel(PointClampSampler, BOTTOMCELL, 0);
    float4 T = VelocityTex1.SampleLevel(PointClampSampler, TOPCELL, 0);
    float4 D = VelocityTex1.SampleLevel(PointClampSampler, DOWNCELL, 0);
    float4 U = VelocityTex1.SampleLevel(PointClampSampler, UPCELL, 0);

    float4 vorticity;
    vorticity.xyz = 0.5 * float3(((T.z - B.z) - (U.y - D.y)) ,
                                 ((U.x - D.x) - (R.z - L.z)) ,
                                 ((R.y - L.y) - (T.x - B.x)));
	vorticity.w = 0.0;
    return vorticity;
}