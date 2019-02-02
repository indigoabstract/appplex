// https://www.shadertoy.com/view/lssGRH
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
	float v = 0.5;
	v += noise2D(p*16.); v*=.5;
	v += noise2D(p*8.); v*=.5;
	v += noise2D(p*4.); v*=.5;
	v += noise2D(p*2.); v*=.5;
	v += noise2D(p*1.); v*=.5;
	return v;
}

vec3 func( vec2  p) {
	p = p*.1+.5;
	vec3 c = vec3(.0, .0, .1);
	vec2 d = vec2(iGlobalTime*.0001, 0.);
	c = mix(c, vec3(.8, .1, .1), pow(fractal(p*.20-d), 3.)*2.);
	c = mix(c, vec3(.9, .6, .6), pow(fractal(p.y*p*.10+d)*1.3, 3.));
	c = mix(c, vec3(1., 1., 1.), pow(fractal(p.y*p*.05+d*2.)*1.2, 1.5));
	return c;
}

void main( void ) {
	vec2 p = 2.*(gl_FragCoord.xy-iResolution.xy*.5)/iResolution.y;
	float d = length(p);
	p *= (acos(d) - 1.57079632)/d;
	gl_FragColor = vec4(func(p)*max(1.-d*d*d, 0.), 1.0);
}
