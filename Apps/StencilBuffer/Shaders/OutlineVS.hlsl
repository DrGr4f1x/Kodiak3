struct VSInput
{
	float4 pos : POSITION;
	float3 normal : NORMAL;
};


cbuffer VSConstants : register(b0)
{
	float4x4 viewProjectionMatrix;
	float4x4 modelMatrix;
	float4 lightPos;
	float outlineWidth;
};


float4 main(VSInput input) : SV_POSITION
{
	float4 pos = float4(input.pos.xyz + input.normal * outlineWidth, input.pos.w);
	pos = mul(viewProjectionMatrix, mul(modelMatrix, pos));

	return pos;
}