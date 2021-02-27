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
    if (IsNonEmptyCell(input.uvw))
        return 0;

    float3 npos = GetAdvectedPosTexCoords(input);

    return ColorTex.SampleLevel(LinearSampler, npos, 0) * modulate;
}