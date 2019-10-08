struct VSInput
{
	float3 position : POSITION;
	float3 normal : NORMAL;
	float3 color : COLOR;
};

#define LIGHT_COUNT 6

struct VSOutput
{
	float4 position : SV_Position;
	float3 normal : NORMAL;
	float3 color : COLOR;
	float4 lightVec[LIGHT_COUNT] : TEXCOORD0;
};

cbuffer VSConstants
{
	float4x4 projectionMatrix;
	float4x4 modelMatrix;
	float4 lightDir[LIGHT_COUNT];
};

struct LightStruct
{
	float4 lightPos[LIGHT_COUNT];
};

#if VK
[[vk::push_constant]]
LightStruct lights;
#else
cbuffer VSLights
{
	LightStruct lights;
};
#endif


VSOutput main(VSInput input)
{
#if VK
	VSOutput output;
#else
	VSOutput output = (VSOutput)0;
#endif

	output.normal = input.normal;
	output.color = input.color;

	float4x4 modelToProjection = mul(projectionMatrix, modelMatrix);
	output.position = mul(modelToProjection, float4(input.position, 1.0f));

	for (int i = 0; i < LIGHT_COUNT; ++i)
	{
		output.lightVec[i].xyz = lights.lightPos[i].xyz - input.position.xyz;
		output.lightVec[i].w = lights.lightPos[i].w;
	}

	return output;
}