// https://www.shadertoy.com/view/MslXD7

// iChannel0: c0
// iChannel1: c1

#define PI 			3.14159265359
#define SPHERE 		vec4 (0.0, 0.0, 0.0, 1.0)
#define FOV 		60.0

#define RI_AIR		1.000293
#define RI_SPH		1.55

#define ETA 		(RI_AIR/RI_SPH)
#define R			-0.02

#define FR_BIAS		0.0
#define FR_SCALE	1.0
#define FR_POWER	0.7

#define FR0			vec3 (0.0, 1.0, 0.7)
#define FR1			vec3 (0.2, 1.0, 0.7)

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

float sphere_intersect (in vec3 o, in vec3 d, in vec4 c, out float t0, out float t1) {
	vec3 oc = o - c.xyz;
	float A = dot (d, d);
	float B = 2.0 * dot (oc, d);
	float C = dot (oc, oc) - c.w;
	float D = B*B - 4.0*A*C;
	float q = (-B - sqrt (D) * sign (B))/2.0;
	float _t0 = q/A;
	float _t1 = C/q;
	t0 = min (_t0, _t1);
	t1 = max (_t0, _t1);
	return step (0.0, D);
}

float fresnel_step (vec3 I, vec3 N, vec3 f) {
	return clamp (f.x + f.y * pow (1.0 + dot (I, N), f.z), 0.0, 1.0);
}

vec4 refraction_sample (vec3 rd, vec3 pt, vec3 pn, vec4 sp, float eta) {
	float t0, t1;
	
	vec3 pf = refract (rd, pn, eta); 		// outer refraction
	
	sphere_intersect (						// find ray exit
		pt, pf, sp, t0, t1);
	
		vec3 ep = pt + pf*t1;				// exit point
	vec3 en = normalize (sp.xyz - ep);		// exit normal
	vec3 er = reflect (pf, en);				// inner reflection
	vec3 ef = refract (pf, en, 1.0/eta);	// inner refraction
	
	return textureCube (iChannel0, ef);
}


void main (void) {
	vec2 uv = (2.0*gl_FragCoord.xy - iResolution.xy)/min (iResolution.x, iResolution.y) * tan (radians (FOV)/2.0);
	vec2 mo = PI * iMouse.xy / iResolution.xy;
	
	vec3 up = vec3 (0.0, 1.0, 0.0); 			// up 
	vec3 fw = vec3 (0.0, 0.0, 1.0) * 			// forward
		rotate_y (mo.x * PI); 				
	vec3 lf = cross (up, fw); 					// left
	
	vec3 ro = -fw * 5.0; 						// ray origin
	vec3 rd = normalize (
		uv.x * lf + uv.y * up + fw) ; 			// ray direction
	vec4 sp = SPHERE + 					
		vec4 (0.0, 1.0, 0.0, 0.0)*
		sin (iGlobalTime); 							
	
	float t0 = 0.0, t1 = 0.0;					// sphere intersection points
	
	float d = sphere_intersect (				// initial intersection
		ro, rd, sp, t0, t1); 
	
	vec4 color;
	
	if (d > 0.0) {
		vec3 pt = ro + rd*min (t0, t1);			// initial entry point
		vec3 pn = normalize (pt - sp.xyz);		// initial point normal
		vec3 pr = reflect (rd, pn);				// outer reflection
		vec4 crefl = mix (
			textureCube (iChannel0, pr),
			textureCube (iChannel1, pr),
			fresnel_step (rd, pn, FR1));
	
		vec4 crefr = vec4 (
			refraction_sample (rd, pt, pn, sp, ETA*(1.0 + R)).r,
			refraction_sample (rd, pt, pn, sp, ETA).g,
			refraction_sample (rd, pt, pn, sp, ETA*(1.0 - R)).b,
			1.0);
		
		color = mix (crefr, crefl, fresnel_step (rd, pn, FR0));
	}
	else {
		color = textureCube (iChannel0, rd);
	}
	gl_FragColor = color;//pow (color, vec4 (1.0/2.0));
}
