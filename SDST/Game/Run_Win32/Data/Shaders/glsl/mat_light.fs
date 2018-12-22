#version 420 core

struct Material
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
	float shininess;
};
uniform Material material;

struct LightMaterial
{
	vec3 ambient;
	vec3 diffuse;
	vec3 spec;
};
uniform LightMaterial light_mat;

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

layout(binding=9, std140) uniform uboTime
{
	float game_time;
	float game_delta_time;
	float app_time;
	float app_delta_time;
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
	// time changing light can be disabled (only for demo purpose)
	//vec3 light_v3 = vec3(lightColor.x, lightColor.y, lightColor.z);
	vec3 light_v3 = vec3(lightColor.x * sin(game_time * 2.0), lightColor.y * sin(game_time * 0.7), lightColor.z * sin(game_time * 1.3));
	vec3 norm = normalize(passNormal);
	vec3 lightDir = normalize(lightPos - passFragPos);

	// ambient
	vec3 ambient_result = (light_v3 * light_mat.ambient) * material.ambient;

	// diffuse
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse_result = (light_v3 * light_mat.diffuse) * (diff * material.diffuse);

	// specular
	vec3 viewDir = normalize(EYE_POSITION - passFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0),material.shininess);
	vec3 specular_result = (light_v3 * light_mat.spec) * (spec * material.specular);

	vec3 result = ambient_result + diffuse_result + specular_result;
	outColor = vec4(result, 1.0);
}
