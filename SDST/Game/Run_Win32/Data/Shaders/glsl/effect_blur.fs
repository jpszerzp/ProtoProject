#version 420 core

// Constants
layout(binding=1, std140) uniform cTimeBlock
{
   float GAME_TIME;
   float GAME_DELTA_TIME;
   float SYSTEM_TIME;
   float SYSTEM_DELTA_TIME; 
}; 

// Suggest always manually setting bindings - again, consitancy with 
// other rendering APIs and well as you can make assumptions in your
// engine without having to query
layout(binding = 0) uniform sampler2D gTexColor;
layout(binding = 1) uniform sampler2D gTexDepth;

in vec2 passUV; 
in vec4 passColor; 

out vec4 outColor; 

int KERNEL_SIZE = 5; 
int OFFSET = 5 / 2; 
float KERNEL[] = {
   0.003765, 0.015019, 0.023792, 0.015019, 0.003765,
   0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
   0.023792, 0.094907, 0.150342, 0.094907, 0.023792,
   0.015019, 0.059912, 0.094907, 0.059912, 0.015019,
   0.003765, 0.015019, 0.023792, 0.015019, 0.003765,
};

// this is transposed (vectors are the columns)
mat4 GRAYSCALE_TRANSFORM = { 
   vec4( 0.2126, 0.7152, 0.0722, 0 ),
   vec4( 0.2126, 0.7152, 0.0722, 0 ),
   vec4( 0.2126, 0.7152, 0.0722, 0 ),
   vec4( 0,      0,      0,      1 )
};

void main( void )
{
   vec4 color = vec4(0); 

   ivec2 pixel_coord = ivec2(gl_FragCoord.xy); 
   for (int y = 0; y < KERNEL_SIZE; ++y) {
      int oy = y - OFFSET; 
      for (int x = 0; x < KERNEL_SIZE; ++x) {
         int ox = x - OFFSET; 

         float weight = KERNEL[y * KERNEL_SIZE + x]; 
         color += weight * texelFetch( gTexColor, pixel_coord + 3 * ivec2(ox, oy), 0 ); 
      }
   }


   vec4 original = texelFetch( gTexColor, pixel_coord, 0 ); 
   vec4 gray = color * GRAYSCALE_TRANSFORM; 
   outColor = mix( gray, original, (sin(SYSTEM_TIME) + 1.0f) * .5f ); 
}
