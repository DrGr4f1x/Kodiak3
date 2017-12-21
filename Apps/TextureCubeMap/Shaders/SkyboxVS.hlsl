struct VSOutput
{
	float4 pos : SV_Position;
	float3 uvw : TEXCOORD;
};


[[vk::binding(0, 0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 viewProjectionMatrix;
	float4x4 modelMatrix;
	float3 eyePos;
}


VSOutput main(float3 pos : POSITION)
{
	VSOutput output = (VSOutput)0;

	output.pos = mul(viewProjectionMatrix, mul(modelMatrix, float4(pos, 1.0f)));

	output.uvw = pos.xyz;
	output.uvw.xy *= -1.0f;

	return output;
}