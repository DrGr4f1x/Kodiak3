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
	float3 normal : NORMAL;
	float3 color : COLOR;
	float3 eyePos : TEXCOORD0;
	float3 lightDir : TEXCOORD1;
	float2 texcoord : TEXCOORD2;
};

cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float gradientPos;
};

VSOutput main(VSInput input)
{
#if VK
	VSOutput output;
#else
	VSOutput output = (VSOutput)0;
#endif

	output.normal = input.normal;
	output.color = input.color;
	output.texcoord = float2(gradientPos, 0.0f);

	float4x4 modelToProjection = mul(projectionMatrix, modelMatrix);
	output.position = mul(modelToProjection, float4(input.position, 1.0f));

	output.eyePos = mul(modelMatrix, float4(input.position, 1.0f)).xyz;

	float3 lightPos = float3(0.0f, 0.0f, -5.0f);
	output.lightDir = normalize(lightPos - input.position);

#if !VK
	output.position.y = -output.position.y;
#endif

	return output;
}