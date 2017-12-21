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

	float3 N = normalize(input.normal);
	float3 L = normalize(input.lightVec);
	float3 V = normalize(input.viewVec);
	float3 R = reflect(-L, N);

	float3 ambient = 0.5f * color.rgb;
	float3 diffuse = max(dot(N, L), 0.0f) * float3(1.0f, 1.0f, 1.0f);
	float3 specular = pow(max(dot(R, V), 0.0f), 16.0f) * float3(0.5f, 0.5f, 0.5f);

	//return float4(ambient + diffuse * color.rgb + specular, 1.0f);
	return color;
}