struct PSInput
{
	float4 pos : SV_Position;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 viewVec : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
};


float4 main(PSInput input) : SV_Target
{
	float3 N = normalize(input.normal);
	float3 L = normalize(input.lightVec);
	float3 V = normalize(input.viewVec);
	float3 R = reflect(-L, N);

	float3 ambient = float3(0.1f, 0.1f, 0.1f);
	float3 diffuse = max(dot(N, L), 0.0f) * float3(1.0f, 1.0f, 1.0f);
	float3 specular = pow(max(dot(R, V), 0.0f), 16.0f) * float3(0.75f, 0.75f, 0.75f);

	return float4((ambient + diffuse) * input.color.rgb + specular, 1.0f);
}