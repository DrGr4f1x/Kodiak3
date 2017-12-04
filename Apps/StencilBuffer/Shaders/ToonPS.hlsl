struct PSInput
{
	float4 pos : SV_Position;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 lightVec : TEXCOORD;
};


float4 main(PSInput input) : SV_TARGET
{
	float3 N = normalize(input.normal);
	float3 L = normalize(input.lightVec);

	float intensity = dot(N, L);
	float3 color = float3(0.0f, 0.0f, 0.0f);

	if (intensity > 0.98f)
		color = input.color * 1.5f;
	else if (intensity > 0.9f)
		color = input.color;
	else if (intensity > 0.5f)
		color = input.color * 0.6f;
	else if (intensity > 0.25f)
		color = input.color * 0.4f;
	else
		color = input.color * 0.2f;

	float3 desatColor = dot(float3(0.2126f, 0.7152f, 0.0722f), color);
	color = lerp(color, desatColor, 0.1f);
	return float4(color, 1.0f);
}