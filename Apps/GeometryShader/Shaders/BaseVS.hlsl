struct VSInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
};


struct VSOutput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.pos = input.pos;
	output.normal = input.normal;

	return output;
}