#version 420 core

#include "inc/common.glsl"
#include "inc/lighting.glsl"
#include "inc/fog.glsl"

// Suggest always manually setting bindings - again, consitancy with 
// other rendering APIs and well as you can make assumptions in your
// engine without having to query
layout(binding = 0) uniform sampler2D gTexDiffuse;

layout(binding=2, std140) uniform uboCamera 
{
   vec3 EYE_POSITION;
   float pad00;
   mat4 VIEW;
   mat4 PROJECTION;  
}; 

in vec3 passViewPos;
in vec3 passWorldPos;
in vec3 passWorldNormal;
in vec3 passWorldTangent; 
in vec3 passWorldBitangent; 

in vec2 passUV; 
in vec4 passColor; 

layout(location = 0) out vec4 outColor; 
layout(location = 1) out vec4 outBloom; 

void main( void )
{
   // Get the surface colour
   vec2 uv_offset = vec2(GAME_TIME * .1f); 
   vec4 tex_color = texture( gTexDiffuse, passUV + uv_offset  ) * passColor; 

   // range map it to a surface normal
   vec3 world_normal = normalize(passWorldNormal);
   vec3 eye_dir = normalize( EYE_POSITION - passWorldPos ); 

   light_factor_t lf = CalculateLighting( passWorldPos, 
      eye_dir, 
      world_normal, 
      SPECULAR_AMOUNT, 
      SPECULAR_POWER ); 

   // Add color of the lit surface to the reflected light
   // to get the final color; 
   //vec4 final_color = vec4(lf.diffuse, 1) * tex_color + vec4(lf.specular, 0);
   vec4 final_color = tex_color;
   outColor = clamp( final_color, vec4(0), vec4(1) ); // not necessary - but overflow should go to bloom target (bloom effect)
   
   //outColor = ApplyFog( outColor, passViewPos.z );
}
