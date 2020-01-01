#version 420 core

#include "inc/common.glsl"

layout(binding=2, std140) uniform uboCamera
{
   vec3 EYE_POSITION;
   float pad00;
   mat4 VIEW;
   mat4 PROJECTION; 
}; 

layout(binding=5, std140) uniform uboObject
{
	mat4 MODEL;
};

// Attributes
in vec3 POSITION;
in vec3 NORMAL; 
in vec4 TANGENT; 

in vec4 COLOR;
in vec2 UV; 

// output
out vec3 passViewPos;
out vec3 passWorldPos;
out vec3 passWorldTangent; 
out vec3 passWorldNormal;
out vec3 passWorldBitangent;

out vec2 passUV; 
out vec4 passColor; 

// Entry Point
void main( void )
{
   vec4 local_pos = vec4( POSITION, 1.0f );  

   vec4 world_pos = MODEL * local_pos; // assume local is world for now; 
   vec4 camera_pos = VIEW * world_pos; 
   passViewPos = camera_pos.xyz; 

   vec4 clip_pos = PROJECTION * camera_pos; 

   passWorldPos = world_pos.xyz; 
   passUV = UV; 
   passColor = COLOR; 

   // get information for tbn space
   passWorldNormal = normalize( (MODEL * vec4( NORMAL, 0.0f )).xyz ); 
   passWorldTangent = normalize( (MODEL * vec4( TANGENT.xyz, 0.0f )).xyz ); 
   passWorldBitangent = normalize( cross( passWorldTangent, passWorldNormal ) * TANGENT.w ); 

   gl_Position = clip_pos; // we pass out a clip coordinate
}
