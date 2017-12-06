struct PSInput
{
	float4 pos : SV_Position;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 eyePos : TEXCOORD1;
	float3 lightVec : TEXCOORD2;
};


Texture2D colorTex : register(t0);
SamplerState linearSampler : register(s0);


float4 main(PSInput input) : SV_Target
{
	//float3 N = normalize(input.normal);
	//float3 L = normalize(float3(1.0f, 1.0f, 1.0f));

	//float3 eyeVec = normalize(input.eyePos);
	//float3 reflected = normalize(reflect(-input.lightVec, input.normal));

	float4 ambient = float4(0.0f, 0.0f, 0.0f, 1.0f);
	float4 diffuse = float4(1.0f, 1.0f, 1.0f, 1.0f) * max(dot(input.normal, input.lightVec), 0.0f);

	return (ambient + diffuse) * colorTex.Sample(linearSampler, input.uv);
}