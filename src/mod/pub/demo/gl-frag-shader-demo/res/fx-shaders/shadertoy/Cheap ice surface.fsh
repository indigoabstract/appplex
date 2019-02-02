// https://www.shadertoy.com/view/4ssSRB

// iChannel0: c0
// iChannel1: c1
// iChannel3: t14

#define PI 			3.14159265359

#define FOV 		60.0
#define EPSILON		0.001
#define MAX_STEPS	1000
#define MAX_DIST	1000.0

#define PLANE		vec4 (0.0, 1.0, 0.0, 1.0)
#define BOTTOM		vec4 (0.0, 1.0, 0.0, 4.0)
#define FR0			vec3 (0.0, 1.5, 5.0)

vec4 texture3D (sampler2D tex, vec3 pt, vec3 pn, float scale) {
	return 
		texture2D (tex, pt.xy/scale) * abs (pn.z) +
		texture2D (tex, pt.xz/scale) * abs (pn.y) +
		texture2D (tex, pt.zy/scale) * abs (pn.x);
}

mat3 rotate_x (float fi) {
	float cfi = cos (fi);
	float sfi = sin (fi);
	return mat3 (
		1.0, 0.0, 0.0,
		0.0, cfi, -sfi,
		0.0, sfi, cfi);
}

mat3 rotate_y (float fi) {
	float cfi = cos (fi);
	float sfi = sin (fi);
	return mat3 (
		cfi, 0.0, sfi,
		0.0, 1.0, 0.0,
		-sfi, 0.0, cfi);
}

mat3 rotate_z (float fi) {
	float cfi = cos (fi);
	float sfi = sin (fi);
	return mat3 (
		cfi, -sfi, 0.0,
		sfi, cfi, 0.0,
		0.0, 0.0, 1.0);
}

vec4 noise3v (vec2 p) {
	return texture2D (iChannel3, p);
}

vec4 fbm3v (vec2 p) {
	vec4 f = vec4 (0.0);
	f += noise3v (p)/pow (2.0, 1.0); p *= 2.01;
	f += noise3v (p)/pow (2.0, 2.0); p *= 2.02;
	f += noise3v (p)/pow (2.0, 3.0); p *= 2.03;
	f += noise3v (p)/pow (2.0, 4.0); p *= 2.04;
	f += noise3v (p)/pow (2.0, 5.0); p *= 2.05;
	f /= 0.9375;
	return f;
}

float dplane (vec3 pt, vec4 pl) {
	return dot (pl.xyz, pt) + pl.w;
}	

float map (vec3 pt) {
	return dplane (pt, PLANE);
}


float march (vec3 ro, vec3 rd) {
	float t = 0.0;
	float d = 0.0;
	vec3 pt = vec3 (0.0);
	for (int i = 0; i < MAX_STEPS; ++i) {
		pt = ro + rd * t;
		d = map (pt); 
		if (d < EPSILON || t + d >= MAX_DIST) {			
			break;
		}
		t += d;
	}
	
	return d <= EPSILON ? t : MAX_DIST;
}

float fresnel_step (vec3 I, vec3 N, vec3 f) {
	return clamp (f.x + f.y * pow (1.0 + dot (I, N), f.z), 0.0, 1.0);
}

float avg (vec3 v) {
	return (v.x + v.y + v.z)/3.0;
}

void main (void) {
	vec2 uv = (2.0*gl_FragCoord.xy - iResolution.xy)/min (iResolution.x, iResolution.y) * tan (radians (FOV)/2.0);
	vec2 mo = PI * iMouse.xy / iResolution.xy;
	
	vec3 up = vec3 (0.0, 1.0, 0.0); 			// up 
	vec3 fw = vec3 (0.0, 0.0, 1.0) * 			// forward
		rotate_y (mo.x * PI); 				
	vec3 lf = cross (up, fw); 					// left
	
	vec3 ro = -fw * 5.0 + vec3 (0.0, 5.0, 0.0); // ray origin
	vec3 rd = normalize (uv.x * lf + uv.y * up + fw) ; 		// ray direction
		
	float t = march (ro, rd);
	vec4 cm = textureCube (iChannel0, rd);
	if (t >= MAX_DIST) {
		gl_FragColor = cm;
	}
	else {
		vec3 pt = rd*t + ro;
		vec4 dv = fbm3v (pt.xz/pow (2.0, 10.0)) - vec4 (0.5);
		//rd += 0.0625*dv.xyz;
		vec3 pn = normalize (PLANE.xyz + 0.0625*dv.xyz) ;		
		vec3 rf = reflect (rd, pn);
		vec3 rr = refract (rd, pn, 1.01);
		vec4 c0 = textureCube (iChannel0, rf);
		vec4 c1 = textureCube (iChannel1, rr);		
		float fs = 1.0-fresnel_step (rd, pn, FR0);
		gl_FragColor = mix (
			mix (mix (c1,c0,1.0 - fs), vec4 (1.0), clamp (avg (dv.xyz+0.3), 0.0, 1.0)),
			cm, smoothstep (60.0, 150.0, t));
	}
}		
