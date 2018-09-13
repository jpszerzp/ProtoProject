#version 420 core

layout(binding=0) uniform sampler2D gTexDiffuse;

layout(binding=7, std140) uniform uboColor
{
    vec4 TINT;
};

in vec4 passColor;
in vec2 passUV;

// Outputs
out vec4 outColor; 

// Entry Point
void main()
{
   vec4 diffuse = texture( gTexDiffuse, passUV );
   vec4 finalColor = diffuse * TINT;

   outColor = finalColor; 
}
