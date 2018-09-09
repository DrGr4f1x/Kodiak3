[[vk::binding(0, 0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 modelViewMatrix;
}


struct VSInput
{
	float3 pos : POSITION;
	float3 color : COLOR;
};


struct VSOutput
{
	float4 pos : SV_POSITION;
	float3 color : COLOR;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.pos = mul(projectionMatrix, mul(modelViewMatrix, float4(input.pos, 1.0)));
	output.color = input.color;

	return output;
}