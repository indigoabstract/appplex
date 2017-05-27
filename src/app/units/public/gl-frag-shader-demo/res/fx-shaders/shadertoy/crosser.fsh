// https://www.shadertoy.com/view/4ssXRn

// iChannel0: m2

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float time = iGlobalTime;
	vec4 t = texture2D(iChannel0,uv);
	float c = sin(t.r+1.0*uv.y)*uv.x/atan(t.r*1.0*uv.y+time*0.5)*uv.x*time*0.01;
	c+=abs(tan(sin(uv.x+abs(cos(time*0.1)*sin(time*0.003))*2.3)+cos(uv.y+cos(time)*sin(time*0.4)*0.5))*uv.x*2.5*t.x*0.01+time*0.5);
	c-=abs(tan(sin(uv.y+abs(sin(time*0.1)*sin(time*0.004))*3.1)+cos(uv.x+sin(time)*cos(time*0.4)*0.5))*uv.x*2.5*t.x*0.01+time*0.5);
	gl_FragColor = vec4(c,c,c,1.0);
}
