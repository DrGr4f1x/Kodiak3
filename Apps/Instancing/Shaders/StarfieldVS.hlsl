struct VSOutput
{
	float4 pos : SV_Position;
	float3 uvw : TEXCOORD0;
};


VSOutput main(uint vertId : SV_VertexID)
{
	VSOutput output = (VSOutput)0;

	output.uvw = float3((vertId << 1) & 2, vertId & 2, vertId & 2);
	output.pos = float4(output.uvw.xy * 2.0 - 1.0, 0.0, 1.0);

	return output;
}