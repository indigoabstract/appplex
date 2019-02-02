// https://www.shadertoy.com/view/XdX3D7

// iChannel0: t1

float perspective = 0.3;

void main(void)
{
	vec2 p = gl_FragCoord.xy / iResolution.xy;
	float focus = sin(iGlobalTime*2.)*.35+.5;
	float blur = 7.*sqrt(abs(p.y - focus));
	
	/* perpective version */	
	//vec2 p2 = vec2(p.x-(1.-p.y)*perspective*(p.x*2. - 1.), -p.y);
	
	/* simple vesion */
	vec2 p2 = -p;	
	
	gl_FragColor = texture2D(iChannel0, p2, blur);
}
