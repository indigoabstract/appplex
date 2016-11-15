// https://www.shadertoy.com/view/4sfXD4
#define E 40.0
void main(void) {
	vec2 uv = (gl_FragCoord.xy * 2.0 - iResolution.xy)/ min (iResolution.x, iResolution.y) ;
	
	float t0 = sin (iGlobalTime);
	float t1 = sin (iGlobalTime/2.0);
	float t2 = cos (iGlobalTime);
	float t3 = cos (iGlobalTime/2.0);
	
	vec2 p0 = vec2 (t1, t0) ;
	vec2 p1 = vec2 (t2, t3) ;
	vec2 p2 = vec2 (t0, t3) ;
	
	float a = 1.0/distance (uv, p0);
	float b = 1.0/distance (uv, p1);
	float c = 1.0/distance (uv, p2);
	
	float d = 1.0 - pow (1.0/(a+b+c), E)*pow (10.0, E*0.7);
	
	gl_FragColor = vec4 (a*0.5, b*0.5, c*0.5, 1.0) * d;	
}
