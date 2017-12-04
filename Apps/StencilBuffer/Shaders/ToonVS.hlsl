struct VSInput
{
	float3 pos : POSITION;
	float3 color : COLOR;
	float3 normal : NORMAL;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 lightVec : TEXCOORD;
};


cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4 lightPos;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.color = float3(1.0f, 0.0f, 0.0f);

	output.pos = mul(projectionMatrix, mul(modelMatrix, float4(input.pos, 1.0f)));
	output.normal = mul((float3x3)modelMatrix, input.normal);

	float4 pos = mul(modelMatrix, float4(input.pos, 1.0f));
	float3 lPos = mul(modelMatrix, lightPos).xyz;
	output.lightVec = lPos - pos.xyz;

	return output;
}