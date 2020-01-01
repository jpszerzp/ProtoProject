#version 420 core

layout(binding=2, std140) uniform uboCamera
{
   vec3 EYE_POSITION;
   float pad00;
   mat4 VIEW;
   mat4 PROJECTION; 
}; 

layout(binding=3, std140) uniform uboSingleLight
{
	vec3 lightPos;
	float pad01;
	vec4 lightColor;
};

layout(binding=7, std140) uniform uboColor
{
    vec4 TINT;
};

in vec2 passUV;
in vec4 passColor;
in vec3 passNormal;
in vec3 passFragPos;

out vec4 outColor;

//uniform vec4 lightColor;

// Entry Point
void main()
{
	// light and color preparation
	vec3 light_v3 = vec3(lightColor.x, lightColor.y, lightColor.z);
	vec3 norm = normalize(passNormal);
	vec3 lightDir = normalize(lightPos - passFragPos);
	vec3 tinted = vec3(TINT.x * passColor.x, TINT.y * passColor.y, TINT.z * passColor.z);

	// ambient
	float ambientStrength = 0.1;
	vec3 ambientLight = ambientStrength * light_v3;
	vec3 ambient_result = ambientLight * tinted;

	// diffuse
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = diff * light_v3;
	vec3 diffuse_result = diffuse * tinted;

	// specular
	float specularStrength = 0.5;
	vec3 viewDir = normalize(EYE_POSITION - passFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0),256);
	vec3 specular = specularStrength * spec * light_v3;
	vec3 specular_result = specular * tinted;

	vec3 result = ambient_result + diffuse_result + specular_result;
	outColor = vec4(result, 1.0);
}
