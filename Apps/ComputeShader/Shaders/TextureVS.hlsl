struct VSInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD;
};


[[vk::binding(0, 0)]]
cbuffer VSConstants
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.uv = input.uv;
	output.pos = mul(projectionMatrix, mul(modelMatrix, float4(input.pos, 1.0f)));

	return output;
}