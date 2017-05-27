// https://www.shadertoy.com/view/4ssSWS
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	

	float a = 3.8;
	float val = abs(a*uv.x*uv.x - a*uv.x + uv.y);
	gl_FragColor.rgb = vec3(0.0, 1.0, 1.0) / (val*200.0);
	gl_FragColor.a += (1.0 - val*200.0);
	

	float x = mod(iGlobalTime*3., 10.) / 10.;
	float y = -a*x*x + a*x;
	vec2 pLight = vec2(x,y);
	
	vec2 q = uv - pLight;
	float len = length(q);
			
	float al = 0.02 / len - .5;
	al = clamp(al, 0.0, 1.0);
		
	gl_FragColor.r += al;
	gl_FragColor.a += al;
	
}
