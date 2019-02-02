// https://www.shadertoy.com/view/4dSXzm
float performance = 0.15; // 0-1, smaller for better accuracy

mat3 rot;

float distanceFrom(in vec3 p) {
    p = mod(p+2.0,4.0)-2.0; // repeat every 2m
    float s = 0.8;
    
    p *= rot; // rotate
    vec3 mountains = sin(p*10.) * sin(iGlobalTime*0.3) * 0.8;
    
    return length(max(abs(p)+length(mountains)-s,0.0));
}

// calculate approximate normal vector
vec3 calcNormal(in vec3 p) {
    vec2 e = vec2(0.001,0);
    return normalize(
        vec3(distanceFrom(p+e.xyy) - distanceFrom(p-e.xyy),
             distanceFrom(p+e.yxy) - distanceFrom(p-e.yxy),
             distanceFrom(p+e.yyx) - distanceFrom(p-e.yyx)));
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 p = -1. + 2.0*uv;
    p.x *= iResolution.x/iResolution.y;
    
    float r = iGlobalTime;
    rot = mat3(
    	1,      0,       0,
    	0, cos(r), -sin(r),
   		0, sin(r),  cos(r));
    vec3 rayOrigin = vec3(0,0,2);
    vec3 rayDirection = normalize(vec3(p, -1.0));
    
    
    float nearestDistance = 1.0;
    float traveled = 0.0;
    float maxTravel = min(iGlobalTime,60.);
    for(int i=0;i<10000;i++) {
        if(nearestDistance < 0.0001 || traveled > maxTravel) break;
        nearestDistance = distanceFrom(rayOrigin + traveled*rayDirection);
        traveled += nearestDistance*performance;
    }
    
    vec3 lightingDirection = normalize(vec3(1));
    vec3 color = vec3(0,0,0);
    
    if(traveled < maxTravel) {
        vec3 pos = rayOrigin + traveled*rayDirection;
        vec3 nor = calcNormal(pos);
        // lighting
        color += vec3(1,0.8,0.5) * clamp(dot(nor,lightingDirection),0.0,1.0);
        color += vec3(0.2,0.3,0.4) * clamp(nor.y,0.0,1.0);
        color += 0.1;
    }
        
    
    gl_FragColor = vec4(color, 1);
}
