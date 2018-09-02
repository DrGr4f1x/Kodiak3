struct VSInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 normal : NORMAL;
	float3 viewVec : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
	float3 reflected : TEXCOORD2;
};


[[vk::binding(0, 0)]]
cbuffer VSConstants
{
	float4x4 viewProjectionMatrix;
	float4x4 modelMatrix;
	float3 eyePos;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	float4 pos = mul(modelMatrix, float4(input.pos, 1.0f));
	
	output.pos = mul(viewProjectionMatrix, pos);
	output.normal = mul((float3x3)modelMatrix, input.normal);

	float3 lightPos = float3(0.0f, -5.0f, 5.0f);
	output.lightVec = lightPos - pos.xyz;
	output.viewVec = -pos.xyz;

	float3 incident = normalize(normalize(pos.xyz) - eyePos);
	output.reflected = reflect(incident, output.normal);

	return output;
}