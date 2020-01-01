#version 420 core

struct Material
{
	//vec3 ambient;
	//vec3 diffuse;
	sampler2D diffuse;
	//vec3 specular;
	sampler2D specular;
	float shininess;
};
uniform Material material;

struct LightMaterial
{
	vec3 direction;
	vec3 pos;
	float cutoff;
	float outerCutoff;

	vec3 ambient;
	vec3 diffuse;
	vec3 spec;

	float constant;
	float linear;
	float quadratic;
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

float ComputeAttenuation(LightMaterial light, float dist)
{
	float attenuation = 1.0 / (light.constant + light.linear * dist + light.quadratic * (dist * dist));
	return attenuation;
}

void ComputePointLight()
{
	// light and color preparation
	// time changing light can be disabled (only for demo purpose)
	vec3 light_v3 = vec3(lightColor.x, lightColor.y, lightColor.z);
	//vec3 light_v3 = vec3(lightColor.x * sin(game_time * 2.0), lightColor.y * sin(game_time * 0.7), lightColor.z * sin(game_time * 1.3));
	vec3 norm = normalize(passNormal);
	vec3 lightDir = normalize(light_mat.pos - passFragPos);
	float dist = length(light_mat.pos - passFragPos);
	float attenuation = ComputeAttenuation(light_mat, dist);

	// ambient
	//vec3 ambient_result = (light_v3 * light_mat.ambient) * material.ambient;
	vec3 ambient_result = (light_v3 * light_mat.ambient) * vec3(texture(material.diffuse, passUV));
	ambient_result *= attenuation;

	// diffuse
	float diff = max(dot(norm, lightDir), 0.0);
	//vec3 diffuse_result = (light_v3 * light_mat.diffuse) * (diff * material.diffuse);
	vec3 diffuse_result = (light_v3 * light_mat.diffuse) * (diff * vec3(texture(material.diffuse, passUV)));
	diffuse_result *= attenuation;

	// specular
	vec3 viewDir = normalize(EYE_POSITION - passFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0),material.shininess);
	//vec3 specular_result = (light_v3 * light_mat.spec) * (spec * material.specular);
	vec3 specular_result = (light_v3 * light_mat.spec) * (spec * vec3(texture(material.specular, passUV)));
	specular_result *= attenuation;

	vec3 result = ambient_result + diffuse_result + specular_result;
	outColor = vec4(result, 1.0);
}

void ComputeDirectionalLight()
{
	vec3 light_v3 = vec3(lightColor.x, lightColor.y, lightColor.z);
	vec3 norm = normalize(passNormal);	
	vec3 lightDir = normalize(-light_mat.direction);

	// ambient
	vec3 ambient_result = (light_v3 * light_mat.ambient) * vec3(texture(material.diffuse, passUV));

	// diffuse
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse_result = (light_v3 * light_mat.diffuse) * (diff * vec3(texture(material.diffuse, passUV)));

	// specular
	vec3 viewDir = normalize(EYE_POSITION - passFragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0),material.shininess);
	vec3 specular_result = (light_v3 * light_mat.spec) * (spec * vec3(texture(material.specular, passUV)));

	vec3 result = ambient_result + diffuse_result + specular_result;
	outColor = vec4(result, 1.0);
}

void ComputeSpotLight()
{
	vec3 light_v3 = vec3(lightColor.x, lightColor.y, lightColor.z);
	vec3 norm = normalize(passNormal);
	vec3 lightDir = normalize(light_mat.pos - passFragPos);
	float dist = length(light_mat.pos - passFragPos);
	float attenuation = ComputeAttenuation(light_mat, dist);
	float theta = dot(lightDir, normalize(-light_mat.direction));
	float epsilon = light_mat.cutoff - light_mat.outerCutoff;
	float intensity = clamp((theta - light_mat.outerCutoff) / epsilon, 0.0, 1.0);

	if (theta > light_mat.outerCutoff)
	{
		// ambient
		vec3 ambient_result = (light_v3 * light_mat.ambient) * vec3(texture(material.diffuse, passUV));
		ambient_result *= attenuation;

		// diffuse
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse_result = (light_v3 * light_mat.diffuse) * (diff * vec3(texture(material.diffuse, passUV)));
		diffuse_result *= attenuation;
		diffuse_result *= intensity;

		// spec
		vec3 viewDir = normalize(EYE_POSITION - passFragPos);
		vec3 reflectDir = reflect(-lightDir, norm);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0),material.shininess);
		vec3 specular_result = (light_v3 * light_mat.spec) * (spec * vec3(texture(material.specular, passUV)));
		specular_result *= attenuation;
		specular_result *= intensity;

		vec3 result = ambient_result + diffuse_result + specular_result;
		outColor = vec4(result, 1.0);
	}
	else
	{
		vec3 ambient_result = (light_v3 * light_mat.ambient) * vec3(texture(material.diffuse, passUV));
		ambient_result *= attenuation;

		outColor = vec4(ambient_result, 1.0);
	}
}

// Entry Point
void main()
{
	ComputePointLight();
	//ComputeDirectionalLight();
	//ComputeSpotLight();
}
