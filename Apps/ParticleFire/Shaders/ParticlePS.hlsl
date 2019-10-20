//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

struct PSInput
{
	float4 pos : SV_Position;
	float4 color : TEXCOORD0;
	float alpha : TEXCOORD1;
	float rotation : TEXCOORD2;
	int type : TEXCOORD3;
	float2 pointSize : TEXCOORD4;
};


[[vk::binding(0, 1)]]
Texture2D texSmoke : register(t0);

[[vk::binding(1, 1)]]
Texture2D texFire : register(t1);

[[vk::binding(0, 2)]]
SamplerState linearSampler : register(s0);


float4 main(PSInput input) : SV_Target
{
	float alpha = (input.alpha <= 1.0f) ? input.alpha : 2.0f - input.alpha;

	// Rotate texture coordinates
	float rotSin, rotCos;
	sincos(input.rotation, rotSin, rotCos);
	float rotCenter = 0.5f;

	float2 rotUV = float2(
		rotCos * (input.pointSize.x - rotCenter) + rotSin * (input.pointSize.y - rotCenter) + rotCenter,
		rotCos * (input.pointSize.y - rotCenter) - rotSin * (input.pointSize.x - rotCenter) + rotCenter);

	// Fire
	if (input.type == 0)
	{
		float3 color = texFire.Sample(linearSampler, rotUV).rgb;
		return float4(color * input.color.rgb * alpha, 0.0f);
	}
	else
	{
		float4 color = texSmoke.Sample(linearSampler, rotUV);
		return float4(color.rgb * input.color.rgb * alpha, color.a * alpha);
	}
}