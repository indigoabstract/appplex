// https://www.shadertoy.com/view/MdjSDD

// iChannel0: t13
// iChannel1: t15

#define EPSILON 	0.005
#define MAXDIST 	100.0
#define MAXSTEPS	100
#define FOV			60.0
#define MSAA

mat3 rotate_x (float fi) {
	float cfi = cos (fi);
	float sfi = sin (fi);
	return mat3 (
		vec3 (1.0, 0.0, 0.0),
		vec3 (0.0, cfi, -sfi),
		vec3 (0.0, sfi, cfi)
	);
}

mat3 rotate_y (float fi) {
	float cfi = cos (fi);
	float sfi = sin (fi);
	return mat3 (
		vec3 (cfi, 0.0, sfi),
		vec3 (0.0, 1.0, 0.0),
		vec3 (-sfi, 0.0, cfi)
	);
}

mat3 rotate_z (float fi) {
	float cfi = cos (fi);
	float sfi = sin (fi);
	return mat3 (
		vec3 (cfi, -sfi, 0.0),
		vec3 (sfi, cfi, 0.0),
		vec3 (0.0, 0.0, 1.0)
	);
}

vec4 tex3d (sampler2D t, vec3 pt, vec3 pn, float scale) {
	return 
		texture2D  (t, pt.yz*scale)*abs (pn.x) +
		texture2D  (t, pt.xz*scale)*abs (pn.y) +
		texture2D  (t, pt.xy*scale)*abs (pn.z);
}

float map (vec3 p, float t) {
    vec3 b = vec3 (0.5, 2.0, 0.5);
    float r = 0.1;
 	return length(max(abs(p)-b,0.0))-r; //length(max(abs(p) - b, 0.0));
}

vec3 normal (vec3 p, float t) {
    float d = map (p, t);
    return normalize (vec3 (
        map (p - vec3 (EPSILON, 0.0, 0.0), t) - d,
        map (p - vec3 (0.0, EPSILON, 0.0), t) - d,
        map (p - vec3 (0.0, 0.0, EPSILON), t) - d
    ));
}

float march (vec3 ro, vec3 rd, float time) {
 	float d = EPSILON;
    float t = 0.0;
    
    for (int i = 0; i < MAXSTEPS; ++i) {
     	vec3 p = ro + rd * d;
       	t = map (p, time);
        if (t < EPSILON || d >= MAXDIST) 
            break;
        d += t;
    }
    return d;
    
}

vec3 hsv2rgb (vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec4 frame (vec2 uv, float t) {
    uv *= tan (radians (FOV)/2.0);
	vec3 up = vec3 (0.0, 1.0, 0.0) ;
	vec3 fw = vec3 (1.0, 0.0, 0.0) * rotate_y (t);
	vec3 rt = cross (fw, up); 
    
    vec3 rd = normalize (uv.y * up + uv.x * rt + fw);
    vec3 ro = -5.0*fw;
    
    
    float d = march (ro, rd, t);
    vec3 p = ro + d*rd;
    vec3 nm = normal (p, t);
    
    float dl = max (dot (nm, fw), 0.0);
 
        
    if (d < MAXDIST) {
     	return vec4 (hsv2rgb (vec3 (tex3d (iChannel0, p, nm, 0.25).r, 1.0, 1.0))*pow (dl,2.0),1.0);
    }
    
    return vec4 (1.0);
}

void main (void) {
    vec2 uv = (iResolution.xy - 2.0*gl_FragCoord.xy)/
		min (iResolution.x, iResolution.y);
    
    float t = iGlobalTime + cos (uv.y)*sin (iGlobalTime)*uv.y*4.0;
    float d = 0.5/min (iResolution.x, iResolution.y);
    #ifdef MSAA
	gl_FragColor = 0.125*(
        frame (uv+vec2(+d, 0.0), t)+
        frame (uv+vec2(-d, 0.0), t)+
        frame (uv+vec2(0.0, +d), t)+
        frame (uv+vec2(0.0, -d), t)+
        frame (uv+vec2(+d*2.0, 0.0), t)+
        frame (uv+vec2(-d*2.0, 0.0), t)+
        frame (uv+vec2(0.0, +d*2.0), t)+
        frame (uv+vec2(0.0, -d*2.0), t)
   );
    #else
	gl_FragColor = frame (uv, t);
    #endif        
    
}
