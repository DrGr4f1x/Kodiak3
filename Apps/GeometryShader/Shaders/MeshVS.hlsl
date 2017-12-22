struct VSInput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float3 color : COLOR;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 viewVec : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
};


cbuffer VSConstants
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.normal = mul((float3x3)modelMatrix, input.normal);
	output.color = input.color;

	float4 pos = mul(modelMatrix, float4(input.pos.xyz, 1.0f));

	float3 lightPos = float3(1.0f, -1.0f, 1.0f);
	output.lightVec = lightPos - pos.xyz;
	output.viewVec = -pos.xyz;

	output.pos = mul(projectionMatrix, pos);

	return output;
}