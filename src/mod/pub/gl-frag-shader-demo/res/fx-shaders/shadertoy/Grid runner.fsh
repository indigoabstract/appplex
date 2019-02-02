// https://www.shadertoy.com/view/XdXGRM
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy - 0.5;
	float r = 3.0+1.0*sin(iGlobalTime*0.0);
	vec2 pos = vec2(sin(iGlobalTime*0.2)*0.3, sin(iGlobalTime*0.5)*0.5);
	
	uv.x *= sin(uv.y*0.1+iGlobalTime);
	
	float distort = -0.5+0.1*sin(iGlobalTime);
	float dist = (distance(pos,uv))*13.14;
	float dx = (10.0+distort*(sin(uv.x*5.0)+dist));
	float dy = 10.0+distort*(sin(uv.y*5.0)+dist)*(uv.y+0.5);
	
	float d = 5.0*pow(0.125/distance(pos,uv),2.0);

	float j = sin(uv.y*r*dy+iGlobalTime*10.0);
	float i = sin(uv.x*r*dx+iGlobalTime*10.0);
	
	float p = clamp(i,0.0,0.2)*clamp(j,0.0,0.2);
	float n = -clamp(i,-0.2,0.0)-0.0*clamp(j,-0.2,0.0);

	float noise = 1.5*(sin(uv.y*10000.0)+5.0*(0.5-abs(uv.x)));
	
	gl_FragColor = noise*(5.0*(vec4(1.0,0.25,0.125,1.0)*n + vec4(1.0,1.0,1.0,1.0)*p)*d);
}
