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
    if (texNumber == 1)
        return abs(ColorTex.SampleLevel(LinearSampler,input.uvw, 0)).xxxx;
    else if (texNumber == 2)
        return abs(VelocityTex0.SampleLevel(LinearSampler,input.uvw, 0));
    else
        return float4(abs(ObstVelocityTex.SampleLevel(LinearSampler,input.uvw, 0).xy),
            abs(ObstaclesTex.SampleLevel(LinearSampler,input.uvw, 0).r),1);
}