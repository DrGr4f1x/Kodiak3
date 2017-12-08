struct PSInput
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD;
};


[[vk::binding(0, 1)]]
Texture2D colorTex : register(t0);
[[vk::binding(0, 2)]]
SamplerState linearSampler : register(s0);


float4 main(PSInput input) : SV_Target
{
	return colorTex.Sample(linearSampler, input.uv);
}