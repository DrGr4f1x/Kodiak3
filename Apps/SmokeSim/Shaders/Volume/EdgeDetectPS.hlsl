//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

#include "Volume.hlsli"


float EdgeDetectScalar(float sx, float sy, float threshold)
{
	float dist = (sx * sx + sy * sy);
	float e = (dist > threshold * zFar) ? 1.0 : 0.0;
	return e;
}


float4 main(VSToPSEdgeDetect input) : SV_TARGET
{
	float4 color = RayDataTexSmall.Sample(PointClampSampler, input.uv00);
	float g00 = color.a;
	if (color.g < 0.0)
		g00 *= -1.0;

	color = RayDataTexSmall.Sample(PointClampSampler, input.uv01);
	float g01 = color.a;
	if (color.g < 0.0)
		g01 *= -1.0;

	color = RayDataTexSmall.Sample(PointClampSampler, input.uv02);
	float g02 = color.a;
	if (color.g < 0.0)
		g02 *= -1.0;

	color = RayDataTexSmall.Sample(PointClampSampler, input.uv10);
	float g10 = color.a;
	if (color.g < 0.0)
		g10 *= -1.0;

	color = RayDataTexSmall.Sample(PointClampSampler, input.uv12);
	float g12 = color.a;
	if (color.g < 0.0)
		g12 *= -1.0;

	color = RayDataTexSmall.Sample(PointClampSampler, input.uv20);
	float g20 = color.a;
	if (color.g < 0.0)
		g20 *= -1.0;

	color = RayDataTexSmall.Sample(PointClampSampler, input.uv21);
	float g21 = color.a;
	if (color.g < 0.0)
		g21 *= -1.0;

	color = RayDataTexSmall.Sample(PointClampSampler, input.uv22);
	float g22 = color.a;
	if (color.g < 0.0)
		g22 *= -1.0;

	// Sobel in horizontal direction
	float sx = 0.0;
	sx -= g00;
	sx -= g01 * 2.0;
	sx -= g02;
	sx += g20;
	sx += g21 * 2.0;
	sx += g22;

	// Sobel in vertical direction
	float sy = 0.0;
	sy -= g00;
	sy += g02;
	sy -= g10 * 2.0;
	sy += g12 * 2.0;
	sy -= g20;
	sy += g22;

	float e = EdgeDetectScalar(sx, sy, edgeThreshold);

	return float4(e, e, e, 1.0);
}