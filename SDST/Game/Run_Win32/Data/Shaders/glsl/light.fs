#version 420 core

layout(binding=7, std140) uniform uboColor
{
    vec4 TINT;
};

in vec2 passUV;
in vec4 passColor;

out vec4 outColor;

uniform vec4 lightColor;

// Entry Point
void main()
{
	vec3 light_v3 = vec3(lightColor.x, lightColor.y, lightColor.z);
	float ambientStrength = 0.1;
	vec3 ambientLight = ambientStrength * light_v3;

	vec3 tinted = vec3(TINT.x * passColor.x, TINT.y * passColor.y, TINT.z * passColor.z);
	vec3 result = ambientLight * tinted;
	outColor = vec4(result, 1.0);
}
