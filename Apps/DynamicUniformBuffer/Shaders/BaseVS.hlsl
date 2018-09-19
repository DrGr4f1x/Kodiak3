[[vk::binding(0, 0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 viewMatrix;
};

[[vk::binding(0, 1)]]
cbuffer VSModelConstants : register(b1)
{
	float4x4 modelMatrix;
};


struct VSInput
{
	float3 pos : POSITION;
	float3 color : COLOR;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 color : COLOR;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.color = input.color;
	output.pos = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, float4(input.pos, 1.0))));

	return output;
}