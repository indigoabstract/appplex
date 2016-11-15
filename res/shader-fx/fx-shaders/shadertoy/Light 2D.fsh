// https://www.shadertoy.com/view/XdjGDm
#define STEPS 16
//#define LIGHT_SPEED 1.5
#define LIGHT_SPEED 0.5
#define HARDNESS 2.0

struct ray {
	vec2 t;
	vec2 p;
	vec2 d;
};

float scene (in vec2 p) {
	p -= vec2 (400,250);
	
	float sn = sin (iGlobalTime / 2.0);
	float cs = cos (iGlobalTime / 2.0);
	
	float f1 = dot (vec2 (sn,-cs), p);
	float f2 = dot (vec2 (cs, sn), p);
	float f = max (f1*f1, f2*f2);
	
	//p += vec2 (sn + 0.5, cs) * 200.0;
	p -= iMouse.xy / iResolution.yy * vec2 (500, 500) - vec2 (400,250);
	f = min (f, dot (p, p) * 2.0);
	
	return 1000.0 / (f + 1000.0);
}

ray newRay (in vec2 origin, in vec2 target) {
	ray r;
	
	r.t = target;
	r.p = origin;
	r.d = (target - origin) / float (STEPS);
	
	return r;
}

void rayMarch (inout ray r) {
	r.p += r.d * clamp (HARDNESS - scene (r.p) * HARDNESS * 2.0, 0.0, LIGHT_SPEED);
}

vec3 light (in ray r, in vec3 color) {
	return color / (dot (r.p, r.p) + color);
}

void main(void) {
	vec2 uv = gl_FragCoord.xy / iResolution.yy * vec2 (500, 500);
	
	ray r0 = newRay (uv, vec2 (600, 250));
	ray r1 = newRay (uv, vec2 (200, 250));
	
	for (int i = 0; i < STEPS; i++) {
		rayMarch (r0);
		rayMarch (r1);
	}
	
	r0.p -= r0.t;
	r1.p -= r1.t;
	
	vec3 light1 = light (r0, vec3 (0.3, 0.2, 0.1) * 20000.0);
	vec3 light2 = light (r1, vec3 (0.1, 0.2, 0.3) * 10000.0);
	
	float f = clamp (scene (uv) * 200.0 - 100.0, 0.0, 3.0);
	
	gl_FragColor = vec4 ((light1 + light2) * (1.0 + f), 1.0);
	//gl_FragColor = vec4 (scene (uv));
}
