struct VSInput
{
	float3 position : POSITION;
	float2 texcoord : TEXCOORD;
	float3 color : COLOR;
	float3 normal : NORMAL;
};

struct VSOutput
{
	float4 position : SV_Position;
	float2 texcoord : TEXCOORD;
	float3 color : COLOR;
};

cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float gradientPos;
};

VSOutput main( VSInput input )
{
#if VK
	VSOutput output;
#else
	VSOutput output = (VSOutput)0;
#endif

	output.color = input.color;
	output.texcoord = float2(gradientPos, 0.0f);

	float4x4 modelToProjection = mul(projectionMatrix, modelMatrix);
	output.position = mul(modelToProjection, float4(input.position, 1.0f));

#if !VK
	output.position.y = -output.position.y;
#endif

	return output;
}