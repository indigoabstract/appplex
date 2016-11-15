// https://www.shadertoy.com/view/MdsXW7
// Nocomment distance. Special distance for low resolution shader (or low latency). Note the low values ​​for the iterations and the gradient.
// Created by vincent francois - cyanux/2014
// Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License (CC BY-NC-ND 3.0)
//
// NoComment distance - Special distance for low resolution shader (or low latency)
// Note the low values for the iterations and the gradient.

// tks TekF (normal, gradient)

#define T iGlobalTime

vec3 XY = vec3(4.0 * (iMouse.xy/iResolution.xy - 0.5), 0.0);

float sdNoComment( vec3 p, vec3 h )
{
	return 
		max(abs(abs(p.x) - h.x), (sin(T) + 1.0) * 0.1 + 0.1) *
		max(abs(abs(p.y) - h.y), (sin(T) + 1.0) * 0.1 + 0.1) *
		max(abs(abs(p.z) - h.z), (sin(T) + 1.0) * 0.1 + 0.1);
}

vec3 rY(const vec3 v, const float t) {
	float COS = cos(t);
	float SIN = sin(t);
	return vec3(COS*v.x-SIN*v.z, v.y, SIN*v.x+COS*v.z);
}

float scene(vec3 p) {
	return sdNoComment(rY(p + vec3( 0.0,  0.0, 4.0) + XY, -T), vec3(0.5, 0.5, 0.5));
}

void main(void) {
	vec2 ar = vec2(iResolution.x/iResolution.y, 1.0);
	vec2 uv = ar * (gl_FragCoord.xy / iResolution.xy - 0.5);
	vec3 ro, rd;
	
	ro = vec3(0.0, 0.0, 1.0);
	rd = normalize(vec3(uv.x, uv.y, -1.0));
	
	vec3 gradient;	
	float d;
	
	for(float n = 0.0; n < 128.0; n++) {
		d = scene(ro);	
		if(d < 0.1) {
			gradient.x = scene(ro + vec3(0.01, 0.0, 0.01));
			gradient.y = scene(ro + vec3(0.0, 0.01, 0.01));
			gradient.z = scene(ro + vec3(0.0, 0.0, 0.01));
			break;
		}
		else
			ro += rd * d * 0.5;
	}
	
	gradient = normalize(gradient);
	
	gl_FragColor = vec4(dot(gradient - d,normalize(vec3(1.0, 1.0, -1.0))));
	gl_FragColor *= (16.0 / (ro.x*ro.x+ro.y*ro.y+ro.z*ro.z));
}
