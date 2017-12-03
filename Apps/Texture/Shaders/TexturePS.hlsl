Texture2D gradientTex : register(t0);
SamplerState linearSampler : register(s0);


struct PSInput
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD0;
	float lodBias : TEXCOORD1;
	float3 normal : NORMAL;
	float3 viewVec : TEXCOORD2;
	float3 lightVec : TEXCOORD3;
};


float4 main(PSInput input) : SV_TARGET
{
	float4 color = gradientTex.SampleLevel(linearSampler, input.uv, input.lodBias);

	float3 normal = normalize(input.normal);
	float3 lightVec = normalize(input.lightVec);
	float3 viewVec = normalize(input.viewVec);

	float3 reflectedLightVec = reflect(-lightVec, normal);

	float3 diffuse = max(dot(normal, lightVec), 0.0f) * float3(1.0f, 1.0f, 1.0f);
	float3 specular = pow(max(dot(reflectedLightVec, viewVec), 0.0f), 16.0f) * color.a;

	return float4(diffuse * color.rgb + specular, 1.0f);
}