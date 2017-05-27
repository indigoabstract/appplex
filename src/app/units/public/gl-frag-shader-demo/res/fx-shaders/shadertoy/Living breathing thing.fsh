// https://www.shadertoy.com/view/4ssSWH

// iChannel0: t0
// iChannel1: t9
// iChannel2: t12

#define PI 					3.14159265359
#define EPSILON 			0.02
#define BUMP_FACTOR     	0.01
#define K					(2.0 + 0.5*sin (iGlobalTime*PI*0.7))

#define MIN_STEP			0.001
#define MAX_STEP			100.0

#define MAX_STEPS 			100
#define MAX_SHADOW_STEPS	64

#define MAX_OCCLUSION_STEPS 8
#define MIN_DIST			EPSILON
#define MAX_DIST			50.0
#define BUMP_SCALE_FACTOR	0.2
#define TEX_SCALE_FACTOR	0.1
#define FOV					60.0

#define SHADOW_HARDNESS 	64.0
#define ATTENUATION			0.025
#define AMBIENT				0.5
#define SHINYNESS			50.0
#define SPECULAR			3.0
#define DIFFUSE				3.0
#define OCCLUSION   		5.0

#define FLICKER_SPEED		3.0
#define FLICKER_THRESHOLD	0.7

#define LIGHT_AMBIENT		vec4(1.0,1.0,1.0,1.0)
#define LIGHT_DYNAMIC		vec4(1.0,1.0,1.0,1.0)
//
//#define MSAA4X

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

mat3 rotate (vec3 vPitchYawRoll, float fi) {
	return
		rotate_z (fi * vPitchYawRoll.z) *
		rotate_y (fi * vPitchYawRoll.y) *
		rotate_x (fi * vPitchYawRoll.x);
}


float smin (float a, float b, float k) {
    return -log (exp (-k*a) + exp (-k*b)) / k;
}

float smax (float x, float y, float k) {
	return log (exp (k*x) + exp (k*y)) / k;
}

struct Plane {
	vec3 n;
	float d;	
};

struct Sphere {
	vec3 o;
	float r;	
};

struct Box {
	vec3 o;
	vec3 b;	
};
	
vec4 texture3d (sampler2D t, vec3 p, vec3 n, float scale) {
	return 
		texture2D(t, p.yz * scale) * abs (n.x) +
		texture2D(t, p.xz * scale) * abs (n.y) +
		texture2D(t, p.xy * scale) * abs (n.z);
}

float get_distance_plane (vec3 p, Plane plane) {
	return dot (plane.n,p) + plane.d;
}

float get_distance_sphere (vec3 p, Sphere sphere) {
	vec3 p0 = p - sphere.o;	
	float l0 = length (p0);
	float bump = 0.0;
	if (l0 < sphere.r + BUMP_FACTOR) {
		bump = BUMP_FACTOR * texture3d(iChannel0, p, normalize (p0), BUMP_SCALE_FACTOR).r ;	
	}			
	return length (p0) - sphere.r + bump;
}

float get_distance_box (vec3 p, Box box) {
	float bump = 0.0;
	vec3 p0 = p - box.o;
	
	if (length (p0) < length (box.b)) {
		bump = texture3d(iChannel0, p, p0, BUMP_SCALE_FACTOR).r * BUMP_FACTOR;
	}	
	
 	vec3 d = abs(p0) - box.b;
  	return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0)) + bump;	
}

float dadd (float a, float b) {
	return smin (a, b, K);
}
float dsub (float a, float b) {
	return smax (a, -b, K);
}

float get_distance (vec3 p) {
	
	float s0 = get_distance_sphere (p, Sphere (vec3 (0.0, 2.5, 0.0), 1.0));
	float s1 = get_distance_sphere (p, Sphere (vec3 (1.5, 1.0, 0.0), 1.0));
	float b0 = get_distance_box (p, Box (vec3 (0.0, 1.0, 0.0), vec3 (1.0, 1.0, 1.0)));
	float b1 = get_distance_box (p, Box (vec3 (0.0, -0.5, 0.0), vec3 (3.0, 1.0, 3.0)));
	float p0 = get_distance_plane (p, Plane (normalize (vec3 (0.0, 1.0, 0.0)),0.0));
	return dadd (dadd (dadd (dadd (b0, b1), s0), s1), p0);
}
vec3 get_normal (vec3 p) {
	float d = get_distance (p);
	return normalize (vec3 (
		get_distance (p - vec3 (EPSILON, 0.0, 0.0)) - d,
		get_distance (p - vec3 (0.0, EPSILON, 0.0)) - d,
		get_distance (p - vec3 (0.0, 0.0, EPSILON)) - d
	));
}

vec2 screen_uv () {
	//vec2 mouse = iMouse.xy / iResolution.xy - vec2 (0.5, 0.5);
	return vec2 (2.0*gl_FragCoord.xy - iResolution.xy)/ 
		vec2 (min (iResolution.x, iResolution.y));
}

float ray_march (vec3 org, vec3 dir, out float md) {	
	float h = EPSILON;
	md = MAX_DIST;	
	for (int i = 0;i < MAX_STEPS;++i) {
		float d = get_distance  (dir * h + org);
		md = min (md, d);
		if (d <= EPSILON) 
			return h;
		h += clamp (d, MIN_STEP, MAX_STEP);
		if (h >= MAX_DIST)
			return h;		
	}
	return MAX_DIST;
}

float light_march (vec3 org, vec3 dir, float k){
    float res = 1.0;
	float t = MIN_DIST;
    for (int i =0; i < MAX_SHADOW_STEPS; ++i){
        float h = get_distance (org + dir*t);
        if (h < 0.001)
            return 0.0;
        res = min (res,k*h/t);
        t += h;
		if (t >=MAX_DIST)
			return res;
    }
    return res;
}

float sample_occlusion (vec3 pnt, vec3 nml) {
	float ao = 0.0;
	float di = 1.0 / float (MAX_OCCLUSION_STEPS);
	for (float i = 0.0; i < float(MAX_OCCLUSION_STEPS); i += 1.0) {
		float d = i * di;
		ao += (1.0/exp2 (i))*(d - get_distance (pnt - nml*d));
	}
	return 1.0 - OCCLUSION * ao;
}

float flicker (float o) {
	return step (FLICKER_THRESHOLD, texture2D (iChannel2, vec2 (o, iGlobalTime/(50.0/FLICKER_SPEED))).r);
}

vec4 calc_ray (vec2 uv) {	
	uv *= tan (radians (FOV)/2.0);
	
	mat3 rotcam = rotate_y (PI * sin (iGlobalTime*PI/10.0) / 4.0);
	mat3 rotlit = rotate_y ((PI / 2.0) * sin (iGlobalTime*PI/5.0));
	
	vec3 lo = vec3 (0.0, 5.0, -10.0) * rotlit;
	vec3 org = vec3 (0.0, 3.0, -7.0) * rotcam;
	vec3 up = vec3 (0.0, 1.0, 0.0) ;
	vec3 fw = vec3 (0.0, 0.0, 1.0) * rotcam;
	vec3 rt = -cross (fw, up) ;
	
	
	vec3 dir = normalize (up * uv.y + rt * uv.x + fw);	
	float md = 0.0;
	float hd = ray_march (org, dir, md);

	
	vec3 pnt = org + dir * hd;
	vec3 nml = get_normal (pnt);
	vec3 lds = pnt - lo;
	vec3 ldr = normalize (lds);
	vec3 lrf = reflect (-ldr, nml);
	
	float shadow = light_march (pnt, -ldr, SHADOW_HARDNESS);
	float occlusion = sample_occlusion (pnt, nml)*AMBIENT*flicker(0.2) ;	
	float diffuse = max (0.0, dot (nml, ldr)) ;	
	float attenuation = (1.0/(1.0+ATTENUATION*pow(length (lds),2.0)));
	float specular = diffuse > 0.0 ? pow(max(0.0, dot(dir, lrf)), SHINYNESS) : 0.0;				
	float lighting = clamp ((DIFFUSE*diffuse + SPECULAR*specular)*shadow*attenuation, 0.0, 1.0);
	
	return mix (
		texture3d (iChannel1, pnt, nml, TEX_SCALE_FACTOR)*
			(LIGHT_DYNAMIC*lighting + LIGHT_AMBIENT*occlusion), 
		mix (vec4 (1.0, 0.0, 0.0, 1.0), 
			 vec4 (0.1, 0.0, 0.1, 1.0), 
			 smoothstep (-1.0, 0.1, dot (up, dir))),
		smoothstep (10.0, 20.0, hd));
	
}


void main (void) {
	float dp = 1.0 / min (iResolution.y, iResolution.x);
	vec2 uv = screen_uv ();
	#ifdef MSAA4X
	gl_FragColor = (
		calc_ray (uv+vec2(-dp/2.0, -dp/2.0))+
		calc_ray (uv+vec2(+dp/2.0, +dp/2.0))+
		calc_ray (uv+vec2(+dp/2.0, -dp/2.0))+
		calc_ray (uv+vec2(-dp/2.0, +dp/2.0))) * 0.25;
	#else
	gl_FragColor = calc_ray (uv);
	#endif
	
}
