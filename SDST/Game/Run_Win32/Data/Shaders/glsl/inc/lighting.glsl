#define MAX_LIGHTS 8

//--------------------------------------------------------------------------------------
struct light_t 
{
   vec4 color;    // alpha is intensity

   // data can only cross a 16-byte boundry IF it starts on a 16-byte boundary.
   vec3 position; 
   float uses_shadow; 

   vec3 direction; 
   float direction_factor; 

   vec3 attenuation; 
   float dot_inner_angle; 

   vec3 spec_attenuation; 
   float dot_outer_angle; 

   vec3 pad03;
   float lightflag;

   mat4 shadow_vp; 
}; 

struct light_factor_t 
{
   vec3 diffuse; 
   vec3 specular; 
}; 

layout(binding=3, std140) uniform uboLights
{
	vec4 AMBIENT;
	light_t LIGHTS[MAX_LIGHTS];
};

layout(binding=4, std140) uniform uboLightConsts
{
	float SPECULAR_AMOUNT;		
	float SPECULAR_POWER;		
	vec2 pad02;
};

layout(binding = 11) uniform sampler2DShadow gTexShadow; 

float GetAttenuation( float i, float d, vec3 a )
{
   return i / (a.x + d * a.y + d * d * a.z);
}

//--------------------------------------------------------------------------------------
// return 1 if fully lit, 0 if should be fully in shadow (ignores light)
float GetShadowFactor( vec3 position, vec3 normal, light_t light )
{
   float shadow = light.uses_shadow;
   if (shadow == 0.0f) {
      return 1.0f; 
   }

   // so, we're lit, so we will use the shadow sampler
   float bias_factor = max( dot( light.direction, normal ), 0.0f ); 
   bias_factor = sqrt(1 - (bias_factor * bias_factor)); 
   position -= light.direction * bias_factor * .25f; 

   vec4 clip_pos = light.shadow_vp * vec4(position, 1.0f);
   vec3 ndc_pos = clip_pos.xyz / clip_pos.w; 

   // put from -1 to 1 range to 0 to 1 range
   ndc_pos = (ndc_pos + vec3(1)) * .5f;
   
   // can give this a "little" bias
   // treat every surface as "slightly" closer"
   // returns how many times I'm pass (GL_LESSEQUAL)
   float is_lit = texture( gTexShadow, ndc_pos ).r; 
   // float my_depth = ndc_pos.z; 
   
   // use this to feathre shadows near the border
   float min_uv = min( ndc_pos.x, ndc_pos.y ); 
   float max_uv = max( ndc_pos.x, ndc_pos.y ); 
   float blend = 1.0f - min( smoothstep(0.0f, .05f, min_uv), smoothstep(1.0, .95, max_uv) ); 

   // step returns 0 if nearest_depth is less than my_depth, 1 otherwise.
   // if (nearest_depth) is less (closer) than my depth, that is shadow, so 0 -> shaded, 1 implies light
   // float is_lit = step( my_depth, nearest_depth ); // 

   // scale by shadow amount
   return mix( light.uses_shadow * is_lit, 1.0f, blend );  
}

//--------------------------------------------------------------------------------------
light_factor_t CalculateLightFactor( vec3 position, 
   vec3 eye_dir, 
   vec3 normal, 
   light_t light, 
   float spec_factor, 
   float spec_power )
{
   light_factor_t lf; 

   vec3 light_color = light.color.xyz;

   // get my direction to the light, and distance
   vec3 light_dir = light.position - position; // direction TO the light
   float dist = length(light_dir); 
   light_dir /= dist; 

   float shadowing = GetShadowFactor( position, normal, light ); 

   // 
   vec3 light_forward = normalize(light.direction); 

   // get the power
   float light_power = light.color.w; 

   // figure out how far away angle-wise I am from the forward of the light (useful for spot lights)
   float dot_angle = dot( light_forward, -light_dir ); 

   // falloff for spotlights.
   float angle_attenuation = smoothstep( light.dot_outer_angle, light.dot_inner_angle, dot_angle ); 
   light_power = light_power * angle_attenuation; 

   // get actual direction light is pointing (spotlights point in their "forward", point lights point everywhere (ie, toward the point))
   light_dir = mix(light_dir, -light_forward, light.direction_factor);

   float attenuation = clamp( GetAttenuation( light_power, dist, light.attenuation ), 0, 1 ); 
   float spec_attenuation = GetAttenuation( light_power, dist, light.spec_attenuation );

   // Finally, calculate dot3 lighting
   float dot3 = dot( light_dir, normal ); 
   float diffuse_factor = clamp( attenuation * dot3, 0.0f, 1.0f );

   // specular
   vec3 r = reflect(-light_dir, normal); 
   float spec_amount = max(dot(r, eye_dir), 0.0f); 
   float spec_intensity = (spec_attenuation * spec_factor) * pow(spec_amount, spec_power); 

   lf.diffuse = shadowing * light_color * diffuse_factor;
   lf.specular = shadowing * light_color * spec_intensity; 

   return lf; 
}


//--------------------------------------------------------------------------------------
light_factor_t CalculateLighting( vec3 world_pos, 
   vec3 eye_dir, 
   vec3 normal, 
   float spec_factor, 
   float spec_power ) 
{
   light_factor_t lf; 
   lf.diffuse = AMBIENT.xyz * AMBIENT.w; 
   lf.specular = vec3(0.0f); 

   spec_factor *= SPECULAR_AMOUNT; 
   spec_power *= SPECULAR_POWER; 

   for (uint i = 0; i < MAX_LIGHTS; ++i) {
      light_factor_t l = CalculateLightFactor( world_pos, eye_dir, normal, LIGHTS[i], spec_factor, spec_power ); 
      lf.diffuse += l.diffuse;
      lf.specular += l.specular; 
   }

   lf.diffuse = clamp( lf.diffuse, vec3(0.0f), vec3(1.0f) ); 
   return lf; 
}
