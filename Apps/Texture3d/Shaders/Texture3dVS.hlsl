struct VSInput
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float2 uv : TEXCOORD;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 uv : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewVec : TEXCOORD1;
	float3 lightVec : TEXCOORD2;
};


[[vk::binding(0, 0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 viewProjectionMatrix;
	float4x4 modelMatrix;
	float4 viewPos;
	float depth;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	float3 worldPos = mul(modelMatrix, float4(input.pos, 1.0f)).xyz;
	output.pos = mul(viewProjectionMatrix, mul(modelMatrix, float4(input.pos, 1.0f)));
	output.normal = mul((float3x3)modelMatrix, input.normal);
	output.uv = float3(input.uv, depth);

	float3 lightPos = float3(0.0f, 0.0f, 0.0f);
	float3 lPos = mul((float3x3)modelMatrix, lightPos);
	output.lightVec = lPos - worldPos;
	output.viewVec = viewPos.xyz - worldPos;

	return output;
}