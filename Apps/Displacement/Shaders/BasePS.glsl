#version 450
#pragma shader_stage(fragment)

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(binding = 0, set = 2) uniform texture2D colorTex;
layout(binding = 0, set = 3) uniform sampler linearSampler;

layout(location = 0) in vec3 inNormal;
layout(location = 1) in vec2 inUV;
layout(location = 2) in vec3 inEyePos;
layout(location = 3) in vec3 inLightVec;

layout(location = 0) out vec4 outFragColor;

void main()
{
	vec3 N = normalize(inNormal);
	vec3 L = normalize(vec3(1.0));

	vec3 Eye = normalize(-inEyePos);
	vec3 Reflected = normalize(reflect(-inLightVec, inNormal));

	vec4 IAmbient = vec4(0.0, 0.0, 0.0, 1.0);
	vec4 IDiffuse = vec4(1.0) * max(dot(inNormal, inLightVec), 0.0);

	//outFragColor = vec4((IAmbient + IDiffuse) * vec4(texture(sampler2D(colorTex, linearSampler), inUV).rgb, 1.0));
	outFragColor = vec4(texture(sampler2D(colorTex, linearSampler), inUV).rgb, 1.0);
}