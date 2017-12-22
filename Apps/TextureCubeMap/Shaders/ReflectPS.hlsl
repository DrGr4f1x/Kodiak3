struct PSInput
{
	float4 pos : SV_Position;
	float3 normal : NORMAL;
	float3 viewVec : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
	float3 reflected : TEXCOORD2;
};


[[vk::binding(1, 1)]]
TextureCube texCube : register(t0);
[[vk::binding(0, 2)]]
SamplerState samplerLinear : register(s0);


[[vk::binding(0, 1)]]
cbuffer PSConstants : register(b0)
{
	float lodBias;
};


float4 main(PSInput input) : SV_Target
{
	float3 cR = input.reflected;
	cR.xy *= -1.0f;

	float4 color = texCube.SampleLevel(samplerLinear, cR, lodBias);

	return color;
}