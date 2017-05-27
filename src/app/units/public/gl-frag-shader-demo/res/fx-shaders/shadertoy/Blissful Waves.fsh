// https://www.shadertoy.com/view/4ssSzj
// Full screen it, lay back and relax. Hope you like it :)
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float n = iGlobalTime/2.0;
	float x = uv.x/4.0+n/12.0;
	float y = uv.y/4.0-sin(n/10.0);
	
	float plas=(y*32.0+n*1.5+sin(x*16.0+n*(0.7)-cos(y*22.0-n*1.5+sin(x*12.0+y*22.0+n*1.1-sin(x*32.0))+cos(x*32.0)+n)+cos(x*16.0-n*1.27)*2.0)*8.0-n+sin(x*2.0-n*1.0)*16.0);
	
	float final = abs(cos((plas+n-sin(y+n*2.0)*cos(x+n*0.2)*2.0)/28.0));
	gl_FragColor = vec4(final,final*1.1,final*1.2,1.0);
}
