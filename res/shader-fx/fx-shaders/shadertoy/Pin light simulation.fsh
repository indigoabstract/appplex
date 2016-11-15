// https://www.shadertoy.com/view/MdlSW8

// iChannel0: t0

#define PI 					3.14159265359
#define EPSILON				0.01
#define FOV 				60.0
#define MAX_STEPS   		300
#define MAX_OCCLUSION_STEPS 3
#define MAX_SHADOW_STEPS 	300
#define MAX_DIST			1000.0
#define MIN_DIST    		EPSILON
#define MIN_STEP			EPSILON
#define MAX_STEP			10.0

#define THETA				1.5
#define ATTENUATION			0.125
#define DIFFUSE				1.0
#define SPECULAR			8.0
#define AMBIENT				0.0125
#define OCCLUSION			1.0
#define SHADOW				1.0
#define GLOSS				25.0
#define SHADOW				1.0
#define SHADOW_SOFT		 	16.0
#define CONE_FACTOR			64.0

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

vec4 fTexture3D (sampler2D t, vec3 pt, vec3 pn, float scale) {
	return 
		texture2D  (t, pt.yz*scale)*abs (pn.x) +
		texture2D  (t, pt.xz*scale)*abs (pn.y) +
		texture2D  (t, pt.xy*scale)*abs (pn.z);
}

struct Sphere {
	vec3 o;
	float r;	
};

struct Plane {
	vec3 n;
	float d;	
};
	
struct Box {
	vec3 o;
	vec3 b;
};

float fGetSDistance (vec3 p, Sphere s) {
	vec3 v = p - s.o;
	float d = length (v) - s.r;
	if (d <= EPSILON) {
		d += fTexture3D (iChannel0, v, normalize (v), 0.25).r * 0.04; 
	}
	return d;
}

float fGetPDistance (vec3 p, Plane s) {	
	float d = dot (s.n,p) + s.d;
	if (d <= EPSILON) {
		d += fTexture3D (iChannel0, p, s.n, 0.25).r * 0.04; 
	}
	return d;
}

float fGetBDistance (vec3 p, Box box) {	
	p.y = mod (p.y, box.b.y); 
	vec3 p0 = p - box.o;
 	vec3 d = abs(p0) - box.b;
	float d0 = min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
	if (d0 <= EPSILON) {
		d0 += fTexture3D (iChannel0, p, normalize (p0), 0.25).r * 0.04; 
	}
  	return d0;	
}

float fGetDistanceScene (vec3 p) {
	return 
		min (
		fGetSDistance (p, Sphere(vec3 (0.0, 1.0, 0.0), 1.0)),
		min (
		fGetPDistance (p, Plane(vec3 (0.0, 1.0, 0.0), -0.0)),
		fGetBDistance (p, Box(vec3 (1.0, 1.0, 2.0), vec3 (1.0, 3.0, 1.0)))));
}


vec3 fGetNormal (vec3 p) {
	float d = fGetDistanceScene (p);
	return normalize (vec3 (
		fGetDistanceScene (p - vec3 (EPSILON, 0.0, 0.0)) - d,
		fGetDistanceScene (p - vec3 (0.0, EPSILON, 0.0)) - d,
		fGetDistanceScene (p - vec3 (0.0, 0.0, EPSILON)) - d));
}

float fRayMarch (vec3 ro, vec3 rd) {
	float d = 0.0;
	float h = 0.0;
	for (int i = 0;i < MAX_STEPS;++i){ 
		h = fGetDistanceScene (ro + rd*d);
		d += clamp (h, MIN_STEP, MAX_STEP);
		if (h < EPSILON || d >= MAX_DIST) 
			return d;
	}
	return MAX_DIST;
}

float fShadowMarch (vec3 org, vec3 dir, float k){
    float res = 1.0;
	float t = EPSILON*32.0;
    for (int i =0; i < MAX_SHADOW_STEPS; ++i){
        float h = fGetDistanceScene (org + dir*t);
        if (h <= EPSILON)
            return 0.0;
        res = min (res, k*h/t);
        t += h;//clamp (h, MIN_STEP, MAX_STEP);
		if (t >= MAX_DIST)
			return res;
    }
    return res;
}


float fSampleOcclusion (vec3 pnt, vec3 nml) {
	float ao = 0.0;
	float di = 1.0 / float (MAX_OCCLUSION_STEPS);
	for (float i = 0.0; i < float(MAX_OCCLUSION_STEPS); i += 1.0) {
		float d = i * di;
		ao += (1.0/exp2 (i))*(d - fGetDistanceScene (pnt - nml*d));
	}
	return 1.0 - OCCLUSION * ao;
}


vec4 fComputeLighting (vec3 p, vec3 n, vec3 rd, vec3 lo, vec3 pd) {
	vec3 lv = -(lo - p);
	vec3 ld = normalize (lv);
	vec3 ref = reflect (-ld, n);
	float pindiff   = pow (1.05*max (0.0, dot (ld, pd)),CONE_FACTOR);
	float diffuse 	= DIFFUSE * max (0.0, dot (ld, n));
	float specular 	= SPECULAR * (diffuse > 0.0 ? pow (max (0.0, dot (rd, ref)),GLOSS) : 0.0);
	float attenuate = 1.0/(1.0 + ATTENUATION*pow (length (lv),2.0));
	float ambient  	= AMBIENT ;
	float shadow  	= (1.0 - SHADOW) + SHADOW*fShadowMarch (p, -ld, SHADOW_SOFT);
	float occlusion = (1.0 - OCCLUSION) + OCCLUSION*fSampleOcclusion (p, n);
	return vec4 (1.0)*((diffuse + specular)*attenuate*shadow*pindiff + ambient*occlusion);
}

vec4 fMaterial (vec3 p, vec3 n) {
	return fTexture3D (iChannel0, p, n, 0.25);		
}

vec4 fComputePixel (vec2 uv, float time) {
	uv *= tan (radians (FOV)/2.0) ;
	mat3 rcamy = rotate_y (iGlobalTime*PI/16.0);		
	mat3 rlity = rotate_y (sin (iGlobalTime*PI/10.0)*PI);
	
	vec3 up = vec3 (0.0, 1.0, 0.0) ;
	vec3 fw = vec3 (0.0, 0.0, 1.0) * rcamy;
	vec3 rt = cross (fw, up);
	
	vec3 ro = vec3 (0.0, 2.0, -10.0) * rcamy ;
	vec3 rd = normalize (fw + up*uv.y + rt*uv.x);
	
	float d = fRayMarch (ro, rd);
	vec3 pt = ro + rd*d;
	vec3 pn = fGetNormal (pt);
	
	vec3 lo = vec3 (0.0, 5.0, 5.0) * rlity;
	vec3 pd = -normalize (vec3 (0.0, 0.3, 1.0)) * rlity;
	
	vec4 cl = mix (fMaterial (pt, pn) * fComputeLighting (pt, pn, rd, lo, pd), vec4 (0.0), smoothstep (0.0, 30.0, d));
	return cl ;
}


void main (void) {
	
	gl_FragColor = pow (fComputePixel (
		(2.0 * gl_FragCoord.xy - iResolution.xy) / 
		min (iResolution.x, iResolution.y),
		iGlobalTime
	), vec4 (1.0/2.2)) ;
	
}
