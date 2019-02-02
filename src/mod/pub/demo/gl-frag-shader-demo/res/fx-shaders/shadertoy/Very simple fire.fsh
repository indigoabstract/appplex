// https://www.shadertoy.com/view/MsXGzl
// scale of flame layers
const float SCALE1 = 12.0;
const float SCALE2 = 17.0;
// speed of flame layers
const vec2 CORESPEED1 = vec2(0.08,0.4);
const vec2 CORESPEED2 = vec2(0.77,1.67);
// brightness of flame layers
const float BRIGHT1=1.0;
const float BRIGHT2=0.5;
// flame colors
const vec3 color1 = vec3(0.5, 0.0, 0.0);
const vec3 color2 = vec3(1.0, 0.5, 0.0);
// cloud colours
//const vec3 color1 = vec3(0.2, 0.4, 0.7);
//const vec3 color2 = vec3(0.7, 1.0, 1.0);// https://www.shadertoy.com/view/MsXGzl

// dither level
const float DITHER = 1.0;

float rand(vec2 n) {
	// we use a smaller constant than the usual 43758.5453, because otherwise 
	// the function returns 0 most of the time on some shaders
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 137.585453);
}

void main() {
	vec2 p = gl_FragCoord.xy / iResolution.xx;
	// random offset from current coordinate results in dithering
	vec2 turb = DITHER*vec2(rand(20.0*p),rand(22.2*p));
	vec2 pf1 = floor(SCALE1*p + turb - CORESPEED1*iGlobalTime);
	vec2 pf2 = floor(SCALE2*p + turb - CORESPEED2*iGlobalTime);
	vec3 c = mix(color1, color2, BRIGHT1*rand(pf1) + BRIGHT2*rand(pf2));
	gl_FragColor = vec4(c, 1.0);
}
