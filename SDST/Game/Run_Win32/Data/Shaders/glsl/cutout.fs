#version 420 core

layout(binding=0) uniform sampler2D gTexDiffuse;

layout(binding=7, std140) uniform uboColor
{
    vec4 TINT;
};

in vec4 passColor; 
in vec2 passUV; 

out vec4 outColor; 

void main( void )
{
   vec4 texColor = texture( gTexDiffuse, passUV ); 
   vec4 finalColor = texColor * TINT;

   if (finalColor.a <= .5f) {
      discard; 
   }

   outColor = finalColor; 
}
