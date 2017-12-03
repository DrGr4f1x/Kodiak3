struct PSInput
{
	float4 pos : SV_Position;
	float3 uv : TEXCOORD;
};


Texture2DArray texArray : register(t0);
SamplerState linearSampler : register(s0);


float4 main(PSInput input) : SV_TARGET
{
	return texArray.Sample(linearSampler, input.uv);
}