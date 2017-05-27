// https://www.shadertoy.com/view/4dfXRf

// iChannel0: c2
// iChannel1: c1

// Created by vincent francois - cyanux/2014
// Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License (CC BY-NC-ND 3.0)

#define D 5.0

//#define OBJECT(P) sdBoundedPlane(P, vec2(1.0, 0.5))
//#define OBJECT(P) sdBox(P, vec3(1.5))
//#define OBJECT(P) sdFastOctahedron(P, 0.7)
//#define OBJECT(P) sdHexagon(P, 0.5)
//#define OBJECT(P) sdInfiniteSquarePrism(P, vec2(0.5, 0.7))
//#define OBJECT(P) sdInfiniteHexagonalPrism(P, vec2(0.5, 0.3))
//#define OBJECT(P) sdInfiniteRhombusPrism(P, vec3(0.5))
#define OBJECT(P) sdRhombicuboctahedron(P, vec3(0.7))
//#define OBJECT(P) sdRhombus(P, 0.5)
//#define OBJECT(P) sdSquarePyramid(P, 1.0)
//#define OBJECT(P) sdTriangle(P, vec3(1.0))
//#define OBJECT(P) sdTruncatedCuboctahedron(P, vec3(0.7))
//#define OBJECT(P) sdTruncatedOctahedron(P, vec3(1.0))
//#define OBJECT(P) sdSphere(P, 1.5)

vec2 M = 8.0 * (iMouse.xy / iResolution.xy - 0.5);

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

float sdBox(vec3 p, vec3 r) {
	return
		max(abs(p.x) - r.x, 0.0) +
		max(abs(p.y) - r.y, 0.0) + 
		max(abs(p.z) - r.z, 0.0);
}
float sdBoundedPlane(vec3 p, vec2 r)
{
	return
		max(
			abs(p.x) - r.x + abs(p.z) - r.y,
			abs(p.y - r.y));
}
float sdFastOctahedron(vec3 p, float r) {
	return abs(p.x) - r + abs(p.y) - r + abs(p.z) - r;
}
float sdHexagon(vec3 p, float r) {
	return
		max(
			max(abs(p.x) - r , 0.0) - r +	max(abs(p.y) -r , 0.0) - r,
			abs(p.z - r));
}
float sdInfiniteSquarePrism(vec3 p, vec2 r) {
	return
		max(
			max(abs(p.x) - r.x, 0.0) + max(abs(p.y) - r.y, 0.0),
			abs(p.y) - r.y);
}
float sdInfiniteHexagonalPrism(vec3 p, vec2 r) {
	return
		max(abs(p.x) - r.x, 0.0) - 0.5 * r.x +
		max(abs(p.y) - r.y, 0.0) - 0.5 * r.y;
}
float sdInfiniteRhombusPrism(vec3 p, vec3 r) {
	return
		min(
			max(abs(p.x) - r.x, 0.0) +
			max(abs(p.y) - r.y, 0.0) +
			max(abs(p.z) - r.z, 0.0),
			abs(p.x) - r.x + abs(p.y) - r.y);
}
float sdRhombicuboctahedron(vec3 p, vec3 r) {
	return
		max(abs(p.x) - r.x, 0.0) - 0.5 * r.x +
		max(abs(p.y) - r.y, 0.0) - 0.5 * r.y +
		max(abs(p.z) - r.z, 0.0) - 0.5 * r.z;
}
float sdRhombus(vec3 p, float r) {
	return
		max(
			abs(p.x) - r + abs(p.y) - r + abs(p.z) - r,
			abs(p.x - r));
}
float sdSquarePyramid(vec3 p, float r) {
	return 
		max(
			abs(p.x) - r + abs(p.y) - r + abs(p.z) - r,
			max(r - p.y, 0.0) - 0.5);
}
float sdTriangle(vec3 p, vec3 r) {
	return 
		max(
			max(
				abs(p.x) - r.x + abs(p.y) - r.y + abs(p.z) - r.z,
				max(r.y - p.y - 0.5, 0.0)),
			abs(p.x - r.x));
			
}
float sdTruncatedCuboctahedron(vec3 p, vec3 r) {
	return
		max(
			abs(p.x) - r.x + abs(p.y) - r.y + abs(p.z) - r.z - (abs(r.x)+abs(r.y)+abs(r.z)) * 0.33,
			max(abs(p.x) - r.x, 0.0) - 0.5 * r.x + max(abs(p.y) - r.y, 0.0) - 0.5 * r.y + max(abs(p.z) - r.z, 0.0) - 0.5 * r.z
		);			
}
float sdTruncatedOctahedron(vec3 p, vec3 r) {
	return
		max(
		abs(p.x) - r.x +
		abs(p.y) - r.y + 
		abs(p.z) - r.z,
			max(abs(p.x) - r.x, 0.0) - 0.5 * r.x + max(abs(p.y) - r.y,0.0) - 0.5 * r.y + max(abs(p.z) - r.z,0.0));
}
float sdSphere(vec3 p, float r) {
	return length(p) - r;
}

bool is_dCM = false;

float scene(vec3 p) {
	float dCM = -sdSphere(p, 16.0);	
	p = rY(p, iGlobalTime);	
	float dO = OBJECT(p);
	is_dCM = dCM < dO ? true : false;
	return min(dO, dCM);
}
void main(void) {
	gl_FragColor = vec4(1.0);
	
	vec2 ar = vec2(iResolution.x/iResolution.y, 1.0);
	vec2 uv = ar * (gl_FragCoord.xy / iResolution.xy - 0.5);
	
	vec3 ro = -rY(rX(vec3(0.0, 0.0, D) , M.y), M.x);
	vec3 co = ro;
	vec3 rd = normalize(rY(rX(vec3(uv, 1.0), M.y), M.x));
	
	vec3 g;
	float d;
	
	for(float n = 0.0; n < 200.0; n++) {
		d = scene(ro);
		if(d < 0.01)
			break;
		ro += rd * d * 0.5;
	}
	
	if(is_dCM == true)
	{
		gl_FragColor = textureCube(iChannel0, ro);
		return;
	}

	g = vec3(
		OBJECT(rY(ro + vec3(0.0005, 0.0000, 0.0000), iGlobalTime)),
		OBJECT(rY(ro + vec3(0.0000, 0.0005, 0.0000), iGlobalTime)),
		OBJECT(rY(ro + vec3(0.0000, 0.0000, 0.0005), iGlobalTime)));
	
	g = normalize(g - d);
	
	gl_FragColor = mix(mix(textureCube(iChannel0, reflect(rd, g)),textureCube(iChannel1, reflect(g, rd)), 0.4), vec4(0.0), 0.2);
}
