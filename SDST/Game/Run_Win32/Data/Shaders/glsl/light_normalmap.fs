#version 420 core

#include "inc/fog.glsl"

#define MAX_LIGHTS 8

////////////////////////////////////////// Structs //////////////////////////////////////////
struct sLightData
{
	vec4 lightColor;

	vec3 lightPosition;
	bool usesShadow;

	vec3 direction;
	float directionFactor;

	vec3 attenuation;
	float innerAngle;

	vec3 specAttenuation;
	float outerAngle;

	vec3 pad03;
	float lightflag;

	mat4 shadowVP;
};

struct sLightFactor
{
	vec3 diffuse;
	vec3 specular;
};

////////////////////////////////////////// UBO //////////////////////////////////////////
layout(binding=0) uniform sampler2D gTexDiffuse;
layout(binding=1) uniform sampler2D gNormalMap;

layout(binding=2, std140) uniform uboCamera 
{
   vec3 EYE_POSITION;
   float pad00;
   mat4 VIEW;
   mat4 PROJECTION;  
}; 

layout(binding=3, std140) uniform uboLights
{
	vec4 AMBIENT;
	sLightData LIGHTS[MAX_LIGHTS];
};

layout(binding=4, std140) uniform uboLightConsts
{
	float SPECULAR_AMOUNT;		
	float SPECULAR_POWER;		
	vec2 pad02;
};

layout(binding=6, std140) uniform uboDebugMode 
{
   vec4 MODE; 
};

layout(binding=7, std140) uniform uboColor
{
    vec4 TINT;
};

////////////////////////////////////////// In and Out //////////////////////////////////////////
in vec2 passUV;
in vec4 passColor;
in vec3 passViewPos;
in vec3 passWorldPos;
in vec3 passWorldNormal;
in vec3 passWorldTangent;
in vec3 passWorldBitangent;

// Outputs
out vec4 outColor; 

////////////////////////////////////////// Functions //////////////////////////////////////////
// i - intensity
// d - distance
// a - attenuation factor
float GetAttenuation( float i, float d, vec3 a )
{
   return i / (a.x + d * a.y + d * d * a.z);
}

vec3 WorldNormalToColor( vec3 normal )
{
   return (normal + vec3(1.0f)) * .5f; 
}

float near = 1.0;
float far = 1000.0;
float GetLinearDepth(float depth)
{
	float z = depth * 2.0 - 1.0;
	float linear_depth = (2.0 * near * far) / (far + near - z * (far - near));
	return linear_depth;
}

sLightFactor CalculateLightFactor(vec3 position, 
	vec3 eyeDir,
	vec3 normal,
	sLightData lightsource,
	float specAmount,
	float specPower)
{
	sLightFactor res;

	// Get light dependent data 
	vec3 forward = normalize(lightsource.direction);
	float lightPower = lightsource.lightColor.w;
	vec3 lightColor = lightsource.lightColor.xyz;

	vec3 lightDir = lightsource.lightPosition - position;
	float distance = length(lightDir);
	lightDir /= distance;
	lightDir = mix(lightDir, -forward, lightsource.directionFactor);

	// updated light power considering light attenuation for deviation (not distance)
	float dotAngle = dot(forward, -lightDir);
	float angleAttenuationFactor = smoothstep(lightsource.outerAngle, lightsource.innerAngle, dotAngle);
	lightPower = lightPower * angleAttenuationFactor;

	float diffuseAttenuation = clamp(GetAttenuation(lightPower, distance, lightsource.attenuation), 0, 1);
	float specularAttenuation = clamp(GetAttenuation(lightPower, distance, lightsource.specAttenuation), 0, 1);

	// diffuse with attenuation
	float dot3 = dot(lightDir, normal);
	float diffuseFactor = clamp(diffuseAttenuation * dot3, 0, 1);

	// specular with attenuation
	float reflectedAmount = max(dot(reflect(-lightDir, normal), eyeDir), 0.0f);
	float specularFactor = (specularAttenuation * specAmount) * pow(reflectedAmount, specPower);

	res.diffuse = lightColor * diffuseFactor;
	res.specular = lightColor * specularFactor;

	return res;
}

sLightFactor CalculateLightFactorIdx(vec3 position,
	vec3 eyeDir,
	vec3 normal,
	float specAmount,
	float specPower,
	int idx)
{
	sLightFactor res;
	res.diffuse = AMBIENT.xyz * AMBIENT.z;
	res.specular = vec3(0.0f);

	specAmount *= SPECULAR_AMOUNT;
	specPower *= SPECULAR_POWER;

	sLightFactor resIdx = CalculateLightFactor(position, eyeDir, normal, LIGHTS[idx], specAmount, specPower);
	res.diffuse += resIdx.diffuse;
	res.specular += resIdx.specular;

	res.diffuse = clamp(res.diffuse, vec3(0.0f), vec3(1.0f));
	return res;
}

sLightFactor CalculateLighting(vec3 position,
	vec3 eyeDir,
	vec3 normal,
	float specAmount,
	float specPower)
{
	sLightFactor res;
	res.diffuse = AMBIENT.xyz * AMBIENT.w;
	res.specular = vec3(0.0f);

	specAmount *= SPECULAR_AMOUNT;
	specPower *= SPECULAR_POWER;

	for (uint i = 0; i < MAX_LIGHTS; ++i)
	{
		if (LIGHTS[i].lightflag == 1.0f)
		{
			sLightFactor resLight = CalculateLightFactor(position, eyeDir, normal, LIGHTS[i], specAmount, specPower);
			res.diffuse += resLight.diffuse;
			res.specular += resLight.specular;
		}
	}

	res.diffuse = clamp(res.diffuse, vec3(0.0f), vec3(1.0f));
	return res;
}

// Entry Point
void main()
{
   vec4 diffuse = texture( gTexDiffuse, passUV );
   vec3 normal = texture( gNormalMap, passUV ).xyz;

   vec3 worldVNormal = normalize(passWorldNormal);				// used to calculate TBN
   vec3 worldVTan = normalize(passWorldTangent);
   vec3 worldVBitan = normalize(passWorldBitangent);
   mat3 surfaceToWorld = mat3(worldVTan, worldVBitan, worldVNormal);

   vec3 surfaceNormal = normalize( normal * vec3( 2.0f, 2.0f, 1.0f ) + vec3( -1.0f, -1.0f, 0.0f ) ); 
   vec3 worldNormal = surfaceToWorld * surfaceNormal;			// TBN

   vec3 eyeDir = normalize(EYE_POSITION - passWorldPos);

   //sLightFactor lightFactor = CalculateLightFactorIdx(passWorldPos, eyeDir, worldNormal, SPECULAR_AMOUNT, SPECULAR_POWER, 0);
   sLightFactor lightFactor = CalculateLighting(passWorldPos, eyeDir, worldNormal, SPECULAR_AMOUNT, SPECULAR_POWER);

   // What is purpose of RENDERMIX x, y and z?
   
   vec4 diffuseColor = vec4(lightFactor.diffuse, 1) * diffuse * TINT;
   vec4 specularColor = vec4(lightFactor.specular, 0);
   vec4 finalColor = diffuseColor + specularColor;
   finalColor = clamp(finalColor, vec4(0), vec4(1));

   outColor = finalColor;
   outColor = ApplyFog( outColor, passViewPos.z );

   switch (int(MODE.w)) {
	  case 0: break;
      case 1: outColor = diffuse; break;
      case 2: outColor = vec4(passUV, 0, 1.0f); break;
      case 3: outColor = vec4(normal, 1); break;
      case 4: outColor = vec4(WorldNormalToColor(normalize(worldNormal)), 1); break;
	  case 8: outColor = diffuseColor; break;
	  case 9: outColor = specularColor; break;
	  case 12: outColor = vec4(vec3(gl_FragCoord.z), 1.0); break;
	  case 13: 
	  {
		  float depth = GetLinearDepth(gl_FragCoord.z) / far;
		  outColor = vec4(vec3(depth), 1.0);
	  }
	  break;
	  default: break;
   }
}
