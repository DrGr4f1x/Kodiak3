struct Particle
{
	float2 pos;
	float2 vel;
	float4 gradientPos;
};

[[vk::binding(0, 0)]]
StructuredBuffer<Particle> particles:register(t0);

[[vk::binding(0, 1)]]
cbuffer VSConstants : register(b0)
{
	float2 invTargetSize;
	float pointSize;
};

struct VSOutput
{
	float4 pos : SV_Position;
	float2 uv : TEXCOORD0;
	float gradientU : TEXCOORD1;
};

VSOutput main(uint vertId : SV_VertexId)
{
	VSOutput output = (VSOutput)0;

	uint particleId = vertId / 6;
	uint index = vertId % 6;

	Particle particle = particles[particleId];
	float2 pos = particle.pos;

	if (index == 0)
	{
		output.pos = float4(pos + 0.5 * invTargetSize * float2(-1.0, 1.0), 0.0, 1.0);
		output.uv = float2(0.0, 0.0);
	}
	else if (index == 1 || index == 5)
	{
		output.pos = float4(pos + 0.5 * invTargetSize * float2(-1.0, -1.0), 0.0, 1.0);
		output.uv = float2(0.0, 1.0);
	}
	else if (index == 2 || index == 4)
	{
		output.pos = float4(pos + 0.5 * invTargetSize * float2(1.0, 1.0), 0.0, 1.0);
		output.uv = float2(1.0, 0.0);
	}
	else
	{
		output.pos = float4(pos + 0.5 * invTargetSize * float2(1.0, -1.0), 0.0, 1.0);
		output.uv = float2(1.0, 1.0);
	}

	output.gradientU = particle.gradientPos.x;

	return output;
}