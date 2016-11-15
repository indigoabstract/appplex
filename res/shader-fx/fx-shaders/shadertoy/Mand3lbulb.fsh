// https://www.shadertoy.com/view/4dfSzf

// iChannel0: c4

// Created by vincent francois - cyanux/2014
// Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License (CC BY-NC-ND 3.0)

#define D 2.0

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
// hsv2rgb - iq / https://www.shadertoy.com/view/MsS3Wc
vec3 hsv2rgb( in vec3 c ) {
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
	return c.z * mix( vec3(1.0), rgb, c.y);
}
float sdSphere(vec3 p, float r) {
	return length(p) - r;
}
bool is_dCM = false;
float L;

float scene(vec3 p) {
	float dCM = -sdSphere(p, 16.0);	
	vec3 v=p;
	vec3 c=v;
	float r=0.0;
	float d=1.0;
	float P = 2.0 + abs(sin(iGlobalTime * 0.04)) * 14.0;
	for(float n=0.0;n<=32.0;n++)
	{
		r=length(v);
		if(r>2.0) break;
		float theta=acos(v.z/r) * P;
		float phi =atan(v.y,v.x) * P;
		d=pow(r,P-1.0)*P*d+1.0;
		v=pow(r, P) * vec3(sin(theta) * cos(phi), sin(phi) * sin(theta), cos(theta)) + c;
		L = r;
	}
	float dO = 0.50*log(r)*r/d;	
	is_dCM = dCM < dO ? true : false;
	return min(dO, dCM);
}
void main(void) {
	vec2 ar = vec2(iResolution.x/iResolution.y, 1.0);
	vec2 uv = ar * (gl_FragCoord.xy / iResolution.xy - 0.5);
	vec3 ro = -rY(rX(vec3(0.0, 0.0, D) , M.y), M.x);
	vec3 co = ro;
	vec3 rd = normalize(rY(rX(vec3(uv, 1.0), M.y), M.x));
	float d;
	for(float n = 0.0; n < 200.0; n++) {
		d = scene(ro);
		if(d < 0.002)
			break;
		ro += rd * d * 0.5;
	}
	if(is_dCM == true)
	{
		gl_FragColor = textureCube(iChannel0, ro);
		return;
	}
	vec3 g = vec3(
		scene(ro + vec3(0.002, 0.000, 0.000)),
		scene(ro + vec3(0.000, 0.002, 0.000)),
		scene(ro + vec3(0.000, 0.000, 0.002)));

	g = normalize(g - d);
	vec3 c = hsv2rgb(vec3(dot(ro, ro), 1.0, L * L - 0.7));
	gl_FragColor.rgb = mix(textureCube(iChannel0, reflect(rd, g)).rgb, c, 0.3);
}
