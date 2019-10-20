struct VSInput
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD;
	float3 color : COLOR;
	float3 normal : NORMAL;
};

struct VSOutput
{
	float4 pos : SV_Position;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD0;
	float3 color : COLOR;
	float3 viewVec : TEXCOORD1;
	float3 lightVec : TEXCOORD2;
};

[[vk::binding(0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 viewMatrix;
	float4x4 modelMatrix;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.color = input.color;
	output.uv = input.uv;

	float4x4 modelToProjection = mul(projectionMatrix, mul(viewMatrix, modelMatrix));
	output.pos = mul(modelToProjection, input.pos);

	float3 lightPos = float3(-5.0f, 5.0f, 0.0f);
	float4 pos = mul(viewMatrix, mul(modelMatrix, input.pos));

	output.normal = mul((float3x3)viewMatrix, mul((float3x3)modelMatrix, input.normal));
	output.lightVec = lightPos - pos.xyz;
	output.viewVec = -pos.xyz;

	return output;
}