// https://www.shadertoy.com/view/4dlSR7
#define N 5.*iResolution.y/360.
#define PI 3.14159265359
float t = iGlobalTime;

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.y;
	vec4 mouse = abs(iMouse);
	
	float s = sin(2.*PI*(N)*uv.y),
		  s1 = .5+.5*sign(s),
		  s2 = sign(sin(2.*PI*N*uv.x+PI/2.*s1*t));
	
	float r = .5*(1.+s2);
	if (abs(s)<.1) r = 1.*(1.+pow(sin(.3*t),3.))/2.;
	gl_FragColor = vec4(r);
}
