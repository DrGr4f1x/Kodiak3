struct VSOutput
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD;
};


VSOutput main(uint vertId : SV_VertexID)
{
	VSOutput output = (VSOutput)0;

	output.uv = float2((vertId << 1) & 2, vertId & 2);
	output.pos = float4(output.uv * 2.0 - 1.0, 0.0, 1.0);
	output.uv.y = 1.0 - output.uv.y;

	return output;
}