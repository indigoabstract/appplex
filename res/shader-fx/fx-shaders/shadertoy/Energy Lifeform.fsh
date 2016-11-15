// https://www.shadertoy.com/view/XdBSDh

// iChannel0: t7

#define SPEED       (1.0 / 40.0)
#define SMOOTH_DIST 0.3

#define PI 3.14159265359

void main(void)
{
    // set up our coordinate system
    float aspectRatio = iResolution.x / iResolution.y;
    vec2 uv = ((gl_FragCoord.xy / iResolution.xy) - vec2(0.5,0.5));
    uv.x *= aspectRatio;
    
	// get our distance and angle
    float dist = length(uv);
    float angle = (atan(uv.y, uv.x) + PI) / (2.0 * PI);
    
   	// texture lookup
    vec3 textureDist;
    textureDist  = texture2D(iChannel0, vec2(iGlobalTime * SPEED, angle)).xyz;
    
    // manipulate distance
    textureDist *= 0.4;
    textureDist += 0.5;
    
    // set RGB
	vec3 color = vec3(0.0,0.0,0.0);
    
    if (dist < textureDist.x)
        color.x += smoothstep(0.0,SMOOTH_DIST, textureDist.x - dist);

    if (dist < textureDist.y)
        color.y += smoothstep(0.0,SMOOTH_DIST, textureDist.y - dist);
    
    if (dist < textureDist.z)
        color.z += smoothstep(0.0,SMOOTH_DIST, textureDist.z - dist);
    
    // add a background grid
    if (fract(mod(uv.x,0.1)) < 0.005 || fract(mod(uv.y,0.1)) < 0.005)
        color.y += 0.5;
    else
        color.y += 0.1;
    
    // set final color
	gl_FragColor = vec4(color,1.0);
}
