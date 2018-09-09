struct PSInput
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 viewVec : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
	float visible : TEXCOORD2;
};


float4 main(PSInput input) : SV_TARGET
{
	if (input.visible > 0.0)
	{
		float3 N = normalize(input.normal);
		float3 L = normalize(input.lightVec);
		float3 V = normalize(input.viewVec);
		float3 R = reflect(-L, N);

		float3 diffuse = max(dot(N, L), 0.0) * input.color;
		float3 specular = pow(max(dot(R, V), 0.0), 8.0) * float3(0.75, 0.75, 0.75);

		return float4(diffuse + specular, 1.0);
	}
	else
	{
		return float4(0.1, 0.1, 0.1, 1.0);
	}
}