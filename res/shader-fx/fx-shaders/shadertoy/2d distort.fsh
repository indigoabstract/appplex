// https://www.shadertoy.com/view/MsX3RM
float noise2D(vec2 uv)
{
	uv = fract(uv)*1e3;
	vec2 f = fract(uv);
	uv = floor(uv);
	float v = uv.x+uv.y*1e3;
	vec4 r = vec4(v, v+1., v+1e3, v+1e3+1.);
	r = fract(1e5*sin(r*1e-2));
	f = f*f*(3.0-2.0*f);
	return (mix(mix(r.x, r.y, f.x), mix(r.z, r.w, f.x), f.y));	
}

float fractal(vec2 p) {
	float v = 0.15;
	p += 0.5;
	v += noise2D(p*0.01); v*=.5;
	v += noise2D(p*0.002); v*=.5;
	v += noise2D(p*0.03); v*=.5;
	v += noise2D(p*0.04); v*=.5;
	v += noise2D(p*0.005); v*=.5;
	return v;
}


float t(vec2 uv, float distort, float size)
{
	return 0.5+0.5*(sin(uv.x*size+distort)+cos(uv.y*size+distort));
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy - 0.5;
	vec2 uv0 = uv;
	uv.x += cos(iGlobalTime*0.125);
	uv.y += sin(iGlobalTime*0.125);
	
	float x = t(uv,iGlobalTime*0.25,20.0);
	float y = t(uv,iGlobalTime*0.25,20.0);
	
	gl_FragColor = vec4(2.0+uv0.x,1.5+uv.x,1.25+uv.y,1.0)*fractal(vec2((uv.x*x)+uv0.x,uv.y*y)+uv0.y)*t(vec2(x*uv.x,y*uv.y),iGlobalTime*2.0,5.0);
		//texture2D(iChannel0, vec2((uv.x*x)+uv0.x,uv.y*y)+uv0.y)*t(vec2(x*uv.x,y*uv.y),iGlobalTime*2.0,20.0);
		
}
