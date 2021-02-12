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

    float4 r;
    float3 diff = abs(halfVolumeDim - input.cell0);

    // Must use regular semi-Lagrangian advection instead of BFECC at the volume boundaries
    if ((diff.x > (halfVolumeDim.x - 4)) || (diff.y > (halfVolumeDim.y - 4)) || (diff.z > (halfVolumeDim.z - 4)))
    {
       r = ColorTex.SampleLevel(LinearSampler, npos, 0);
    }
    else
    {
        // Texture_color contains \phi^n; Texture_tempscalar contains \bar{\phi}
        //  (i.e.: the result of 1 forward advection step, followed by a backwards advection step)
        r = 1.5f * ColorTex.SampleLevel(LinearSampler, npos, 0)
            - 0.5f * TempScalarTex.SampleLevel(LinearSampler, npos, 0);
    }

    r = saturate(r);
    return r * modulate;
}