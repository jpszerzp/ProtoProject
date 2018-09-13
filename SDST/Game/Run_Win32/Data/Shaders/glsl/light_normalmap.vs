#version 420 core

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

//uniform mat4 PROJECTION;
//uniform mat4 VIEW;
//uniform mat4 MODEL;

// Attributes - input to this shasder stage (constant as far as the code is concerned)
in vec3 POSITION;
in vec3 NORMAL;
in vec4 TANGENT;
in vec4 COLOR;
in vec2 UV;

out vec2 passUV;
out vec4 passColor;
out vec3 passViewPos;
out vec3 passWorldPos;
out vec3 passWorldNormal;
out vec3 passWorldTangent;
out vec3 passWorldBitangent;

void main()
{
   vec4 local_pos = vec4(POSITION, 1);
   vec4 world_pos = MODEL * local_pos;
   vec4 cam_pos = VIEW * world_pos;
   vec4 clip_pos = PROJECTION * cam_pos;
   passViewPos = cam_pos.xyz; 

   passWorldPos = world_pos.xyz;
   passUV = UV;
   passColor = COLOR;

   // Info for tbn space
   passWorldNormal = normalize((MODEL * vec4(NORMAL, 0.0f)).xyz);
   passWorldTangent = normalize((MODEL * vec4(TANGENT.xyz, 0.0f)).xyz);
   passWorldBitangent = normalize(cross(passWorldTangent, passWorldNormal) * TANGENT.w);
   
   gl_Position = clip_pos; 
}