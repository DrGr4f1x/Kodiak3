//
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
// Author:  David Elder
//

struct VSInput
{
	float3 pos : POSITION;
};


struct VSToPSRayDataBack
{
	float4 pos :	SV_Position;
	float depth :	TEXCOORD0;
};


struct VSToPSRayDataFront
{
	float4 pos :		SV_Position;
	float3 posInGrid :	TEXCOORD0;
	float depth :		TEXCOORD1;
};


struct VSToPSRayCast
{
	float4 pos :		SV_Position;
	float3 posInGrid :	TEXCOORD0;
};


struct VSToPSEdgeDetect
{
	float4 pos :	SV_Position;
	float2 uv00 :	TEXCOORD0;
	float2 uv01 :	TEXCOORD1;
	float2 uv02 :	TEXCOORD2;
	float2 uv10 :	TEXCOORD3;
	float2 uv12 :	TEXCOORD4;
	float2 uv20 :	TEXCOORD5;
	float2 uv21 :	TEXCOORD6;
	float2 uv22 :	TEXCOORD7;
};


[[vk::binding(0, 0)]]
cbuffer RayCastConstants : register(b0)
{
	float4x4 worldProjectionMatrix;
	float4x4 projectionWorldMatrix;
	float3 gridDim;
	float3 invGridDim;
	float3 eyeOnGrid;
	float2 rtDim;
	float2 invRtDim;
	float zNear;
	float zFar;
	float maxGridDim;
	float gridScaleFactor;
	float edgeThreshold;
};


[[vk::binding(0, 1)]]
Texture3D   ColorTex :			register(t0);
[[vk::binding(1, 1)]]
Texture2D   RayDataTex :		register(t1);
[[vk::binding(2, 1)]]
Texture2D   RayDataTexSmall :	register(t2);
[[vk::binding(3, 1)]]
Texture2D   RayCastTex :		register(t3);
[[vk::binding(4, 1)]]
Texture2D   SceneDepthTex :		register(t4);
[[vk::binding(5, 1)]]
Texture2D   EdgeTex :			register(t5);
[[vk::binding(6, 1)]]
Texture2D   JitterTex :			register(t6);


[[vk::binding(0, 2)]]
SamplerState PointClampSampler :	register(s0);
[[vk::binding(1, 2)]]
SamplerState LinearClampSampler :	register(s1);
[[vk::binding(2, 2)]]
SamplerState LinearWrapSampler :	register(s2);


#define OCCLUDED_PIXEL_RAYVALUE     float4(1, 0, 0, 0)
#define NEARCLIPPED_PIXEL_RAYPOS    float3(0, -1, 0)


#define BACK_TO_FRONT 0
void DoSample(float weight, float3 O, inout float4 color)
{
#define OPACITY_MODULATOR 0.1

	float3 uvw = float3(O.x, 1.0 - O.y, O.z);
	float4 samp = weight * ColorTex.SampleLevel(LinearClampSampler, uvw, 0);
	samp.a = samp.r * OPACITY_MODULATOR;

#if BACK_TO_FRONT
	color.rgb = (1.0 - samp.s) * color.r + samp.a * samp.r;
	color.a = (1.0 - samp.a) * color.a + samp.a;
#else
	float t = samp.a * (1.0 - color.a);
	color.rgb += t * samp.r;
	color.a += t;
#endif
}


float4 RayCast(VSToPSRayCast input)
{
	float4 color = 0.0;
	float4 rayData = RayDataTex.Sample(LinearClampSampler, float2(input.pos.xy * invRtDim));

	if (rayData.x < 0.0)
	{
		return color;
	}

	if (rayData.y < 0.0)
	{
		rayData.xyz = input.posInGrid;
		rayData.w = rayData.w - zNear;
	}

	float3 rayOrigin = rayData.xyz;
	float offset = JitterTex.Sample(LinearWrapSampler, input.pos.xy / 256.0).r;
	float rayLength = rayData.w;

	float fSamples = (rayLength / gridScaleFactor * maxGridDim) * 2.0;
	int nSamples = floor(fSamples);
	float3 stepVec = normalize((rayOrigin - eyeOnGrid) * gridDim) * invGridDim * 0.5;

	float3 O = rayOrigin + stepVec * offset;

#if BACK_TO_FRONT
	O += fSample * stepVec;
	stepVec = -stepVec;
#endif

	for (int i = 0; i < nSamples; ++i)
	{
		DoSample(1.0, O, color);
		O += stepVec;

#if !BACK_TO_FRONT
		if (color.a > 0.99)
			break;
#endif
	}

	if (i == nSamples)
	{
		DoSample(frac(fSamples), O, color);
	}

	return color;
}