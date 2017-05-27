// https://www.shadertoy.com/view/lsfSDn

// iChannel0: t3
// iChannel1: t10

#define M_PI (3.14159265359)

void main (void) {

	vec2 gRes = iResolution.xy;
	vec2 gPos = gl_FragCoord.xy/gRes;
	vec2 cRes = iChannelResolution [0].xy;	
	vec2 cPos = (gPos - vec2 (0.5, 0.5))*2.0;
	
	float r = distance (cPos, vec2 (0.0));
	float a = atan (cPos.x, cPos.y) / (M_PI * 2.0);
	
	float r0 = mod (r + iGlobalTime/4.0, 1.0) ;
	float a0 = mod (a + iGlobalTime/8.0 + r, 1.0) ;
	
	float r1 = mod (r*0.125 + iGlobalTime/8.0, 1.0) ;
	float a1 = mod (a, 1.0) ;
	
	vec4 _0 = texture2D(iChannel0, vec2 (r0, a0));
	vec4 _1 = texture2D(iChannel1, vec2 (r1, a1));
	gl_FragColor =  _0 + _1*(1.0 - r)*0.6 + (1.0 - r)*0.7;

}
