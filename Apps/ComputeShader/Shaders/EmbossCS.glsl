#version 450
#pragma shader_stage(compute)

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(local_size_x = 16, local_size_y = 16) in;
layout(binding = 0, rgba8) uniform readonly image2D inputImage;
layout(binding = 1, rgba8) uniform image2D resultImage;

float conv(in float[9] kernel, in float[9] data, in float denom, in float offset)
{
	float res = 0.0;
	for (int i = 0; i<9; ++i)
	{
		res += kernel[i] * data[i];
	}
	return clamp(res / denom + offset, 0.0, 1.0);
}

struct ImageData
{
	float avg[9];
} imageData;

void main()
{
	// Fetch neighbouring texels
	int n = -1;
	for (int i = -1; i<2; ++i)
	{
		for (int j = -1; j<2; ++j)
		{
			n++;
			vec3 rgb = imageLoad(inputImage, ivec2(gl_GlobalInvocationID.x + i, gl_GlobalInvocationID.y + j)).rgb;
			imageData.avg[n] = (rgb.r + rgb.g + rgb.b) / 3.0;
		}
	}

	float[9] kernel;
	kernel[0] = -1.0; kernel[1] = 0.0; kernel[2] = 0.0;
	kernel[3] = 0.0; kernel[4] = -1.0; kernel[5] = 0.0;
	kernel[6] = 0.0; kernel[7] = 0.0; kernel[8] = 2.0;

	vec4 res = vec4(vec3(conv(kernel, imageData.avg, 1.0, 0.50)), 1.0);

	imageStore(resultImage, ivec2(gl_GlobalInvocationID.xy), res);
}