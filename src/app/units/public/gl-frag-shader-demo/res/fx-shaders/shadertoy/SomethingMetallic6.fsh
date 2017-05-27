// https://www.shadertoy.com/view/4ssXzS

// iChannel0: c2

// Valid Truncated Cuboctahedron & gradient values modified (thickness simulation) [Special thanks to TekF for normal and gradient (much better than me in 3d, for sure)]
// Created by vincent francois - cyanux/2014
// Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License (CC BY-NC-ND 3.0)
//
// vfrancois.pro@hotmail.fr
//
// Distances + Phong Shading + Reflection (no iteration)

// Some modifications : normal computation and reflect method from
// https://www.shadertoy.com/view/4dj3zV

// Boolean operator from iq website (tks)
// http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm

// Tks to Lebesgue, Bourbaki group, and many others! (see Lp space)

// Tks to tekf (math), iq (distance, raymarching), rez (the scene is dead, amazing simplicity), 
// dubmood  (best tracks 4 ever) and many others !

// Varying this value!
#define D 5.0 // distance from object

// Choose your object :)

//#define OBJECT sdBoundedPlane(p, vec2(1.0, 0.5))
//#define OBJECT sdBox(p, vec3(1.5))
//#define OBJECT sdFastOctahedron(p, vec3(0.5))
#define OBJECT sdHexagon(p, 0.5)
//#define OBJECT sdInfiniteSquarePrism(p, vec2(0.5, 0.7))
//#define OBJECT sdInfiniteHexagonalPrism(p, vec2(0.5, 0.5))
//#define OBJECT sdInfiniteRhombusPrism(p, vec3(0.5))
//#define OBJECT sdRhombicuboctahedron(p, vec3(0.7))
//#define OBJECT sdRhombus(p, 0.5)
//#define OBJECT sdSquarePyramid(p, 1.0)
//#define OBJECT sdTriangle(p, vec3(1.0))
//#define OBJECT sdTruncatedCuboctahedron(p, vec3(0.7))
//#define OBJECT sdTruncatedOctahedron(p, vec3(1.0))


//#define OBJECT sdSphere(p, 1.5)

// Following is a dirty code

#define T iGlobalTime

vec2 iMouseXY = 8.0 * (iMouse.xy / iResolution.xy - 0.5);
//vec2 iMouseXY = vec2(0.0);

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
float sdFastOctahedron(vec3 p, vec3 r) {
	return abs(p.x) - r.x + abs(p.y) - r.y + abs(p.z) - r.z;
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
	
	float dO = OBJECT;
		
	if(dCM < dO) is_dCM = true;
					 
	return min(dO, dCM);
}
void main(void) {
	gl_FragColor = vec4(1.0);
	
	vec2 ar = vec2(iResolution.x/iResolution.y, 1.0);
	vec2 uv = ar * (gl_FragCoord.xy / iResolution.xy - 0.5);
	
	vec3 ro = -rY(rX(vec3(0.0, 0.0, D) , iMouseXY.y), iMouseXY.x);
	vec3 co = ro;
	vec3 rd = normalize(rY(rX(vec3(uv, 1.0), iMouseXY.y), iMouseXY.x));
	
	vec3 gradient;
	float d;
	
	for(float n = 0.0; n < 128.0; n++) {
		d = scene(ro);
		if(d < 0.01) {
			gradient.x = scene(ro + vec3(0.00001, 0.0,   0.000));
			gradient.y = scene(ro + vec3(0.0,   0.00001, 0.000));
			gradient.z = scene(ro + vec3(0.0,   0.0,   0.00001));
			break;
		}
		else
			ro += rd * d * 0.5;
	}
	
	
	if(is_dCM == true)
	{
		gl_FragColor = textureCube(iChannel0, ro);
		return;
	}
	
	// Light position
	vec3 Lp = rY(vec3(0.0, 0.0, 10.0), T * 4.0);
	// The direction vector from the point on the surface toward each light source
	vec3 L = normalize(Lp - ro);
	// The normal at this point on the surface
	// !!! Valid normal !!!
	vec3 n = normalize(gradient - d);
	// The direction that a perfectly ray of light would take from this point on the
	// surface
	vec3 r = 2.0 * dot(L, n) * n - L;
	// The direction pointing towards the viewer (such as virtual camera)
	vec3 e = r;
	// The direction of reflection
	//vec3 f = 2.0 * dot(normalize(rd), n) * n + normalize(rd);
	// correct reflection vector (from https://www.shadertoy.com/view/4dj3zV)
	vec3 f = reflect(normalize(rd), n);
	// Ambient, Diffuse and Specular components
	vec3 Ca = mix(vec3(0.25),textureCube(iChannel0, f).xyz, 0.9);
	vec3 Cd = mix(vec3(0.4), textureCube(iChannel0, f).xyz, 0.5);
	vec3 Cs = mix(vec3(0.4), textureCube(iChannel0, f).xyz, 0.3);
	// Shininess
	float P  = 0.3;
	// Ambient factor
	float Ka = 0.9;
	// Diffuse factor
	float Kd = 0.1;	
	// Specular factor
	float Ks = 0.001;	
	// Diffuse (Lambertian) Component
	vec3 Ld = Kd * Cd * max(0.0, dot(n, L));	
	// Ambient Component
	vec3 La = Ka * Ca;	
	// Specular (Phong) Component
	vec3 Ls = Ks * Cs * pow(max(0.0, dot(e, r)), P);	
	// Total radiance
	vec3 Ip = (La + Ld + Ls) / length(Lp) * length(Lp);	

	gl_FragColor.xyz = Ip;
}
