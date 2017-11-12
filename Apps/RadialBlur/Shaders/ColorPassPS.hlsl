struct PSInput
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
	float3 color : COLOR;
};

[[vk::binding(0)]]
Texture2D gradientTex : register(t0);
[[vk::binding(0, 1)]]
SamplerState linearSampler : register(s0);

float3 main(PSInput input) : SV_TARGET
{
	// Use max. color channel value to detect bright glow emitters
	if ((input.color.r >= 0.9f) || (input.color.g >= 0.9f) || (input.color.b >= 0.9f))
	{
		return gradientTex.Sample(linearSampler, input.texcoord).rgb;
	}
	else
	{
		return input.color;
	}
}