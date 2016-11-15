// https://www.shadertoy.com/view/4ssGzX
float circle(vec2 uv, float diameter, float speed)
{
	speed *= sin(iGlobalTime*0.3-0.9);
	//speed *= 1.0+0.4*length(uv)*sin(iGlobalTime*0.03);
	float angle = iGlobalTime*speed-3.4*length(uv)*sin(iGlobalTime*0.3);
	uv*= mat2(  sin(angle), cos(angle),
				cos(angle),-sin(angle));
	
	float pixelate = 0.07-sin(iGlobalTime*0.1)*0.045;
	//pixelate -= mod(pixelate,0.001);
	uv -= mod(uv,pixelate)-pixelate*0.5;
	return 1.0-smoothstep(0.0002,0.0001+pixelate,(abs(mod(length(uv),0.05)-diameter)));
}

void main(void)
{
	vec2 uv = (gl_FragCoord.xy-iResolution.xy/2.0) / min(iResolution.x,iResolution.y);
	
	gl_FragColor = vec4(circle(uv +0.012*vec2(sin(iGlobalTime*0.81),sin(iGlobalTime*0.34)), 0.008,  0.023),
						circle(uv +0.012*vec2(sin(iGlobalTime*0.72),sin(-iGlobalTime*0.63)), 0.02,   0.022),
						circle(uv +0.012*vec2(sin(-iGlobalTime*0.623),sin(iGlobalTime*0.53)), 0.032,  0.021),
						1.0);
}
