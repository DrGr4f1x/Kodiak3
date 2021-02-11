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

    float dist = length(input.cell0 - center) * size;
    float4 result;
    result.rgb = splatColor.rgb;    // + sin(splatColor.rgb*10.0+cell*5.0)*0.2;
    result.a = exp(-dist * dist);

    return result;
}