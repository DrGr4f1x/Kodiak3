struct PSInput
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 color : COLOR;
	float3 viewVec : TEXCOORD1;
	float3 lightVec : TEXCOORD2;
};


float4 main(PSInput input) : SV_TARGET
{
	float3 ambient = float3(0.0, 0.0, 0.0);

	if ((input.color.r >= 0.9) || (input.color.g >= 0.9) || (input.color.b >= 0.9))
	{
		ambient = input.color * 0.25;
	}
	
	float3 N = normalize(input.normal);
	float3 L = normalize(input.lightVec);
	float3 V = normalize(input.viewVec);
	float3 R = reflect(-L, N);
	float3 diffuse = max(dot(N, L), 0.0) * input.color;
	float3 specular = pow(max(dot(R, V), 0.0), 8.0) * float3(0.75, 0.75, 0.75);

	return float4(ambient + diffuse + specular, 1.0);
}