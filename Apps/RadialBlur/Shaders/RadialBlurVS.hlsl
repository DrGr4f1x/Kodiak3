struct VSOutput
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD0;
};


VSOutput main( uint vertId : SV_VertexID )
{
#if VK
	VSOutput output;
#else
	VSOutput output = (VSOutput)0;
#endif

	output.texcoord = float2((vertId << 1) & 2, vertId & 2);
	output.position = float4(output.texcoord * 2.0f - 1.0f, 0.0f, 1.0f);

#if !VK
	output.position.y = -output.position.y;
#endif

	return output;
}