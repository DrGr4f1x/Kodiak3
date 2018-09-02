struct VSInput
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BITANGENT;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float3 lightVec : TEXCOORD0;
	float3 lightVec2 : TEXCOORD1;
	float3 lightDir : TEXCOORD2;
	float3 viewVec : TEXCOORD3;
	float2 uv : TEXCOORD4;
};


[[vk::binding(0, 0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 projectionMatrix;
	float4x4 viewMatrix;
	float4x4 modelMatrix;
	float4x4 normalMatrix;
	float4 lightPos;
	float4 cameraPos;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	float3 vertPos = mul(modelMatrix, float4(input.pos, 1.0f)).xyz;
	output.lightDir = normalize(lightPos.xyz - vertPos);

	float3x3 tbnMatrix;
	tbnMatrix[0] = mul((float3x3)normalMatrix, input.tangent);
	tbnMatrix[1] = mul((float3x3)normalMatrix, input.bitangent);
	tbnMatrix[2] = mul((float3x3)normalMatrix, input.normal);

	output.lightVec = mul(tbnMatrix, lightPos.xyz - vertPos);
	float3 lightDist = lightPos.xyz - input.pos;
	output.lightVec2.x = dot(input.tangent, lightDist);
	output.lightVec2.y = dot(input.bitangent, lightDist);
	output.lightVec2.z = dot(input.normal, lightDist);

	output.viewVec.x = dot(input.tangent, input.pos.xyz);
	output.viewVec.y = dot(input.bitangent, input.pos.xyz);
	output.viewVec.z = dot(input.normal, input.pos.xyz);

	output.uv = input.uv;

	output.pos = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, float4(input.pos, 1.0f))));

	return output;
}