struct VSInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD;
	uint instanceId : SV_InstanceId;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 uv : TEXCOORD;
};


struct InstanceData
{
	float4x4 modelMatrix;
	float4 arrayIndex;
};


cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 viewMatrix;
	InstanceData instance[8];
}

VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.uv = float3(input.uv, instance[input.instanceId].arrayIndex.x);

	float4x4 modelToProjection = mul(projectionMatrix, mul(viewMatrix, instance[input.instanceId].modelMatrix));
	output.pos = mul(modelToProjection, float4(input.pos, 1.0));

	output.pos.y *= -1.0f;

	return output;
}