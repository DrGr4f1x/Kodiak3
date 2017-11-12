struct PSInput
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD0;
};

[[vk::binding(0)]]
Texture2D colorTex : register(t0);
[[vk::binding(0,1)]]
SamplerState linearSampler : register(s0);

[[vk::binding(1)]]
cbuffer PSConstants : register(b0)
{
	float radialBlurScale;
	float radialBlurStrength;
	float2 radialOrigin;
};

float4 main(PSInput input) : SV_TARGET
{
	uint2 texDim;
	colorTex.GetDimensions(texDim.x, texDim.y);

	float2 radialSize = float2(1.0f / texDim.x, 1.0f / texDim.y);

	float2 uv = input.texcoord;
	uv += radialSize * 0.5f - radialOrigin;

	float4 color = float4(0.0f, 0.0f, 0.0f, 0.0f);

	const int nSamples = 32;

	for (int i = 0; i < nSamples; i++)
	{
		float scale = 1.0f - radialBlurScale * (float(i) / float(nSamples - 1));
		color += colorTex.Sample(linearSampler, uv * scale + radialOrigin);
	}

	return (color / nSamples) * radialBlurStrength;
}