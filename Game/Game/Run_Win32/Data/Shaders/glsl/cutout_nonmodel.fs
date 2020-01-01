#version 420 core

layout(binding = 0) uniform sampler2D gTexDiffuse;

in vec4 passColor; 
in vec2 passUV; 

out vec4 outColor; 

void main()
{
   vec4 texColor = texture( gTexDiffuse, passUV ); 
   vec4 finalColor = texColor * passColor;

   if (finalColor.a <= .5f) {
      discard; 
   }

   outColor = finalColor; 
}
