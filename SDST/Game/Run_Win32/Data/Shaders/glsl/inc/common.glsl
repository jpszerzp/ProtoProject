// Frame information ;
layout(binding=9, std140) uniform uboGameTime
{
   float GAME_TIME;
   float GAME_DELTA_TIME;
   float SYSTEM_TIME; 
   float SYSTEM_DELTA_TIME; 
}; 

layout(binding=10, std140) uniform uboFogInfo
{
   // Fog
   vec4 FOG_COLOR; 
   float FOG_NEAR_PLANE;
   float FOG_NEAR_FACTOR;
   float FOG_FAR_PLANE;
   float FOG_FAR_FACTOR;
};