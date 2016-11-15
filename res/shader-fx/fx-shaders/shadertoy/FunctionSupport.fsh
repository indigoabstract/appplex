// https://www.shadertoy.com/view/4slSzB
// Another method for plotting function. (TN definition : good value is 0.01 ; 0.5 is fun). Compatible with any functions ! No iteration ! [ <8O~ enabled (support rotation) ]
// Created by vincent francois - cyanux/2014
// Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License (CC BY-NC-ND 3.0)
//
// Plotting function on support
//
// From https://www.shadertoy.com/view/4slSzB
//
// vfrancois.pro@hotmail.fr

#define TN 0.5

vec2 R(vec2 p, float t) {
	float C = cos(t), S = sin(t);
	return vec2(C*p.x-S*p.y,S*p.x+C*p.y);
}
float fn(vec2 A, vec2 B, vec2 U, float t, float f) {
	vec2 a = B - A;
	vec2 i = normalize(vec2(a.y, -a.x)) * f * .5 + a * t + A;
	return dot(i-U,i-U);
}
void main(void) {
	float M = 6.28 * iMouse.x / iResolution.x;
	float T = iGlobalTime * .25;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 A = R(vec2( .5, .0), M) + .5;
	vec2 B = R(vec2(-.5, .0), M) + .5;

	float t = dot(B - A, uv - A);
	float d = 1024. * fn(A, B, uv, t, sin(T + t * 12.) * sin(T + t * 5.));

	uv = R(uv - 0.5, -M);
		   
	if(d < TN && d < .95)
		gl_FragColor = vec4(d);
	else if(mod(uv.x, .1) < .005 || mod(uv.y, .1) < .005)
		gl_FragColor = vec4(1., 0., 0., 0.);
	else
		gl_FragColor = vec4(1.);
}
