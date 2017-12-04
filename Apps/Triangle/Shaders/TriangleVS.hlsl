cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4x4 viewMatrix;
};

struct VSInput
{
	float3 pos : POSITION;
	float3 color : COLOR;
};

struct VSOutput
{
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.color = input.color;
	float4x4 modelToProjection = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
	output.position = mul(modelToProjection, float4(input.pos, 1.0));

	return output;
}