// https://www.shadertoy.com/view/ldsXD7
// Nanotechnology (sometimes shortened to "nanotech") is the manipulation of matter on an atomic, molecular, and supramolecular scale. In center, you see an electronic field! [Mouse enabled]
// Created by vincent francois - cyanux/2014
// Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License (CC BY-NC-ND 3.0)
//
// NanoTek
//
// Repetition operator from iq website

// Varying this value!
#define D 8.0 // distance to electronic field
// Varying this value between 0.0 and 0.5
#define DOT_CORRECTION 0.01

// Following is a dirty code

#define T iGlobalTime

float iMouseY = 4.0 * (iMouse.y / iResolution.y - 0.5);
float COS = cos(T);
float SIN = sin(T);

vec3 rX(vec3 v, float t) {
	float COS = cos(t);
	float SIN = sin(t);
	return vec3(v.x,SIN*v.z+COS*v.y,COS*v.z-SIN*v.y);
}
vec3 rY(const vec3 v, const float t) {
	float COS = cos(t);
	float SIN = sin(t);
	return vec3(COS*v.x-SIN*v.z, v.y, SIN*v.x+COS*v.z);
}
float sdHPI(vec3 p, vec3 r) {
	return
		max(abs(p.x) - r.x, 0.0) - 0.5 * r.x +
		max(abs(p.y) - r.y, 0.0) - 0.5 * r.y;
}
float sdNoComment( vec3 p, vec3 h ) {
	return
		max(abs(abs(p.x) - h.x), (SIN + 1.0) * 0.1) *
		max(abs(abs(p.y) - h.y), (SIN + 1.0) * 0.1) *
		max(abs(abs(p.z) - h.z), (SIN + 1.0) * 0.1);
}
float sdRhombi(vec3 p, vec3 r) {
	return
		max(abs(p.x) - r.x, 0.0) - 0.5 * r.x +
		max(abs(p.y) - r.y, 0.0) - 0.5 * r.y +
		max(abs(p.z) - r.z, 0.0) - 0.5 * r.z;
}
float scene(vec3 p) {
	float d1 = sdRhombi(mod(p, vec3(2.0)) - 0.5 * vec3(2.0), vec3(0.1, 0.1, 0.1));
	float d2 = sdNoComment(p, vec3(0.1, 0.1, 0.1));
	//float d3 = sdHPI(p + vec3(0.0 ,0.0, 0.0), vec3(0.1, 0.1, 0.1));
	float d3 = sdHPI(mod(p, vec3(4.0)) - 0.5 * vec3(4.0), vec3(0.1, 0.1, 0.1));
	float d4 = sdHPI(mod(rY(p - vec3(1.0), 1.57), vec3(4.0)) - 0.5 * vec3(4.0), vec3(0.1, 0.1, 0.1));
	
	d1 = min(min(min(d1, d2), d3), d4);

	if(d1 == d2) {
		gl_FragColor = vec4(abs(cos(T)), 1.0, abs(sin(T)), 0.0);
		return d2;
	}
	gl_FragColor = vec4(1.0);
	return d1;
}
void main(void) {
	vec2 ar = vec2(iResolution.x/iResolution.y, 1.0);
	vec2 uv = ar * (gl_FragCoord.xy / iResolution.xy - 0.5);
	vec3 ro = -rY(rX(vec3(0.0, 0.0, D) , iMouseY), T);
	vec3 rd = rY(rX(vec3(uv, 1.0), iMouseY), T);
	
	vec3 gradient;
	float d;
	
	for(float n = 0.0; n < 32.0; n++) {
		d = scene(ro);
		if(d < 0.001) {
			gradient.x = scene(ro + vec3(0.001, 0.0,   0.0));
			gradient.y = scene(ro + vec3(0.0,   0.001, 0.0));
			gradient.z = scene(ro + vec3(0.0,   0.0,   0.001));
			break;
		}
		else
			ro += rd * d * 0.5;
	}
		
	vec3 lp = vec3(0.0, 0.0, 1.0);
	//float ld = DOT_CORRECTION + 0.5 * abs(dot(normalize(gradient), lp));
	float ld = 1.0 - max(dot(normalize(gradient - d), lp), 0.0);

	if(d < 0.0 ) ld = 0.0;
	
	if(length(ro) > 16.0)
	{
		gl_FragColor = vec4(0.0);
		return;
	}
	
	gl_FragColor *= 0.1 * vec4(ld) * (-abs(SIN * 8.0) +  16.0) /  length(lp - ro);
}
