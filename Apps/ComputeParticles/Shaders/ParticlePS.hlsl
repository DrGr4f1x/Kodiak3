[[vk::binding(0, 0)]]
Texture2D colorTex : register(t0);
[[vk::binding(1, 0)]]
Texture2D gradientTex : register(t1);

[[vk::binding(0, 1)]]
SamplerState linearSampler : register(s0);


struct PSInput
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD0;
	float gradientU : TEXCOORD1;
};


float4 main(PSInput input) : SV_TARGET
{
	float3 color = colorTex.Sample(linearSampler, input.uv).rgb;
	color *= gradientTex.Sample(linearSampler, input.gradientU).rgb;

	return float4(color, 1.0);
}