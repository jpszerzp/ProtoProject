#include "inc/common.glsl"

vec4 ApplyFog( vec4 color, float view_depth )
{
    float fog_factor = smoothstep( FOG_NEAR_PLANE, FOG_FAR_PLANE, view_depth ); 
    fog_factor = FOG_NEAR_FACTOR + (FOG_FAR_FACTOR - FOG_NEAR_FACTOR) * fog_factor; 
    color = mix( color, FOG_COLOR, fog_factor ); 

    return color; 
}