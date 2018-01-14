struct PSInput
{
	float4 pos : SV_Position;
	float3 uv : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewVec : TEXCOORD1;
	float3 lightVec : TEXCOORD2;
};


[[vk::binding(0, 1)]]
Texture3D texColor : register(t0);
[[vk::binding(0, 2)]]
SamplerState linearSampler : register(s0);


float4 main(PSInput input) : SV_Target
{
	float3 color = texColor.Sample(linearSampler, input.uv).rgb;

	return float4(color, 1.0f);
}