// https://www.shadertoy.com/view/4dfSW4
#define MAX_SAMPLES 200
#define THICKNESS	0.1
#define BLUR		20.0
#define FREQ		200.0
#define PI			3.14159265359


vec2 fBezier2 (vec2 p0, vec2 p1, vec2 p2, float t) {
	return mix (
		mix (p0, p1, t), 
		mix (p1, p2, t),t);	
}

vec2 fBezier3 (vec2 p0, vec2 p1, vec2 p2, vec2 p3, float t) {
	return fBezier2 (
		mix (p0, p1, t),
		mix (p1, p2, t),
		mix (p2, p3, t),t);		
}
	

float fGetDistance (vec2 p, vec2 p0, vec2 p1, vec2 p2, vec2 p3) {
	float d0 = distance (p, p0) ;
	for (int i = 0;i < MAX_SAMPLES;++i) {
		float d = distance (p, fBezier3 (p0, p1, p2, p3, float (i)/float (MAX_SAMPLES)));
		d0 = min (d0, d);			
	}
	return d0;
}
	
void main (void) {
	float t = iGlobalTime;
	vec2 uv = (2.0 * gl_FragCoord.xy - iResolution.xy)/ min (iResolution.x, iResolution.y);
	
	vec2 p1 = vec2 (-0.5, 0.0);
	vec2 p2 = vec2 (0.5, 0.0);
	vec2 p0 = vec2 (sin (t), cos (t))*0.5 + p1;
	vec2 p3 = vec2 (cos (t), sin (t))*0.5 + p2;
	
	float d = fGetDistance (uv, p0, p1, p2, p3);
	float s = smoothstep (THICKNESS*(1.0 - BLUR), THICKNESS*(1.0 + BLUR), d);
	
	gl_FragColor = mix (vec4 (0.0), vec4 (1.0), 0.5 + 0.5*sin (s*PI*FREQ) );
							
	
}
