struct VSOutput
{
	float4 position : SV_POSITION;
	float3 color : COLOR;
};

float4 main(VSOutput input) : SV_TARGET
{
	return float4(input.color, 1.0f);
}