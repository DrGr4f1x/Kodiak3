struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD;
	float3 color : COLOR;
};

struct VSOutput
{
	float4 position : SV_Position;
	float3 color : COLOR;
	float3 normal : NORMAL;
	float2 texcoord : TEXCOORD0;
	float3 lightVec : TEXCOORD1;
	float3 viewVec : TEXCOORD2;
};

[[vk::binding(0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4 lightPos;
};

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.normal = mul((float3x3)modelMatrix, input.normal);
	output.texcoord = input.texcoord;
	output.color = input.color;

	output.position = mul(projectionMatrix, mul(modelMatrix, float4(input.position, 1.0)));

	float4 pos = mul(modelMatrix, float4(input.position, 1.0));
	float3 lPos = mul((float3x3)modelMatrix, lightPos.xyz);
	output.lightVec = lPos - pos.xyz;
	output.viewVec = -pos.xyz;

	return output;
}