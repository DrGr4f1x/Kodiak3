struct VSInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};


struct VSOutput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.pos = float4(input.pos, 1.0f);
	output.uv = input.uv * 3.0f;
	output.normal = input.normal;

	return output;
}