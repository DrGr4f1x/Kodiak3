struct PSInput
{
	float4 pos : SV_Position;
	float3 lightVec : TEXCOORD0;
	float3 lightVec2 : TEXCOORD1;
	float3 lightDir : TEXCOORD2;
	float3 viewVec : TEXCOORD3;
	float2 uv : TEXCOORD4;
};


// TODO - Put this shit in a cbuffer
#define lightRadius 45.0f


[[vk::binding(0, 1)]]
Texture2D texColor : register(t0);
[[vk::binding(1, 1)]]
Texture2D texNormal : register(t1);

[[vk::binding(0, 2)]]
SamplerState linearSampler : register(s0);


float4 main(PSInput input) : SV_Target
{
	float3 specColor = float3(0.85f, 0.5f, 0.0f);

	float invRadius = 1.0f / lightRadius;
	float ambient = 0.25f;

	float3 color = texColor.Sample(linearSampler, input.uv).rgb;
	float3 normal = normalize((texNormal.Sample(linearSampler, input.uv).rgb - 0.5f) * 2.0f);

	float distSqr = dot(input.lightVec2, input.lightVec2);
	float3 lVec = input.lightVec2 * rsqrt(distSqr);

	float atten = max(clamp(1.0f - invRadius * sqrt(distSqr), 0.0f, 1.0f), ambient);
	float diffuse = clamp(dot(lVec, normal), 0.0f, 1.0f);

	float3 light = normalize(-input.lightVec);
	float3 view = normalize(input.viewVec);
	float3 reflectDir = reflect(-light, normal);

	float specular = pow(max(dot(view, reflectDir), 0.0f), 4.0f);

	return float4((color * atten + (diffuse * color + 0.5f * specular * specColor)) * atten, 1.0f);
}