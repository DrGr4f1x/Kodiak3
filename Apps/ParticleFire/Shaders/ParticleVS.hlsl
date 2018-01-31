struct VSInput
{
	float4 pos : POSITION;
	float4 color : COLOR;
	float alpha : TEXCOORD0;
	float size : TEXCOORD1;
	float rotation : TEXCOORD2;
	int type : TEXCOORD3;
};


struct VSOutput
{
	float4 pos : SV_Position;
	float4 color : TEXCOORD0;
	float alpha : TEXCOORD1;
	float rotation : TEXCOORD2;
	int type : TEXCOORD3;
	float2 pointSize : TEXCOORD4;
};


[[vk::binding(0, 0)]]
cbuffer VSConstants : register(b0)
{
	float4x4 modelMatrix;
	float4x4 viewMatrix;
	float4x4 projectionMatrix;
	float2 screenSize;
	float pointSize;
};


VSOutput main(VSInput input)
{
	VSOutput output = (VSOutput)0;

	output.color = input.color;
	output.alpha = input.alpha;
	output.rotation = input.rotation;
	output.type = input.type;

	output.pos = mul(projectionMatrix, mul(viewMatrix, mul(modelMatrix, input.pos)));

	// Base size of the point sprite
	float spriteSize = 8.0f * input.size;

	float4 eyePos = mul(viewMatrix, mul(modelMatrix, float4(input.pos.xyz, 1.0f)));
	float4 projectedCorner = mul(projectionMatrix, float4(0.5f * spriteSize, 0.5f * spriteSize, eyePos.z, eyePos.w));

	float ps = screenSize.x * projectedCorner.x / projectedCorner.w;
	output.pointSize = float2(ps, ps);

	return output;
}