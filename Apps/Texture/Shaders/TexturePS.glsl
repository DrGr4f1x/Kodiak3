#version 450
#pragma shader_stage(fragment)

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0, set = 1) uniform texture2D colorTex;
layout(binding = 0, set = 2) uniform sampler linearSampler;

layout(location = 0) in vec2 inUV;
layout(location = 1) in float inLodBias;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inViewVec;
layout(location = 4) in vec3 inLightVec;

layout(location = 0) out vec4 outFragColor;

void main()
{
	vec4 color = texture(sampler2D(colorTex, linearSampler), inUV, inLodBias);

	vec3 N = normalize(inNormal);
	vec3 L = normalize(inLightVec);
	vec3 V = normalize(inViewVec);
	vec3 R = reflect(-L, N);
	vec3 diffuse = max(dot(N, L), 0.0) * vec3(1.0);
	float specular = pow(max(dot(R, V), 0.0), 16.0) * color.a;

	outFragColor = vec4(diffuse * color.rgb + specular, 1.0);
}