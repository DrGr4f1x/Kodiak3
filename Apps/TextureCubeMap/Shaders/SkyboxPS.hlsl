struct PSInput
{
	float4 pos : SV_Position;
	float3 uvw : TEXCOORD;
};


[[vk::binding(0, 1)]]
TextureCube texCube : register(t0);
[[vk::binding(0, 2)]]
SamplerState samplerLinear : register(s0);


float4 main(PSInput input) : SV_Target
{
	return texCube.Sample(samplerLinear, input.uvw);
}