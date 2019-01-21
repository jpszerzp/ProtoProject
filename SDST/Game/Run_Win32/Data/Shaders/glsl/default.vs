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
    mat4 INV_MODEL;
};

// Attributes - input to this shader stage (constant as far as the code is concerned)
in vec3 POSITION;
in vec4 COLOR;
in vec2 UV;

out vec2 passUV;
out vec4 passColor;

void main()
{
   vec4 local_pos = vec4(POSITION, 1);
   vec4 clip_pos = PROJECTION * VIEW * MODEL * local_pos;

   passColor = COLOR;       // vert color
   passUV = UV;
   gl_Position = clip_pos; 
}
