// https://www.shadertoy.com/view/XdXSWN

// iChannel0: t15

// Demonstrating how polar mapping can be derived without costly trigonometric functions, if you avoid dealing in angles.
void main(void)
{
	vec2 p = (gl_FragCoord.xy / iResolution.xy)*2.0-1.0;	
	p.x *= iResolution.x/iResolution.y;
	
	// radius
	float r = length(p);
	
	// these three lines equivalent to sin(atan(x,y)),cos(atan(x,y))
	float u = p.x/p.y;
	float squu = sqrt(1.0 + u*u);
	vec2 sc = vec2(u,1.0) / (squu*mix(1.0,-1.0,step(p.y,0.0)));
	
	vec2 uv = 0.5 + 0.5*sc;
	
	gl_FragColor = vec4(vec3(uv, r), 1.0);
}
