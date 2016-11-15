// https://www.shadertoy.com/view/4sfGz8
// retro wallpaper construction kit
// @simesgreen

const float PI = 3.1415926535;
const float TWOPI = PI*2.0;

const vec3 blue = vec3(0.0, 0.5, 1.0);
const vec3 yellow = vec3(1.0, 1.0, 0.0);
const vec3 orange = vec3(1.0, 0.5, 0.0);
const vec3 purple = vec3(1.0, 0.0, 0.5);
const vec3 white = vec3(1.0, 1.0, 1.0);
const vec3 black = vec3(0.0, 0.0, 0.0);

float flower(vec2 p, vec2 c, float r, float n, float a)
{
	p -= c;
	float t = atan(p.y, p.x); // + time;
	float d = length(p);
	d += sin(t*n)*a;
	//return smoothstep(r, r-0.01, d);
	return d;
}

float flower2(vec2 p, vec2 c, float r, float n, float a)
{
	p -= c;
	float t = atan(p.y, p.x);
	float d = length(p);
	d -= abs(sin(t*n))*a;
	//return smoothstep(r, r-0.01, d);
	return d;
}

float dots(vec2 p, vec2 c, float r, float n)
{
	p -= c;
	float t = (atan(p.y, p.x) + PI) / TWOPI;
	t = floor(t * n) / n; // integer part
	t += 0.5 / n;
	t = (t*TWOPI)-PI;
	vec2 dc = vec2(cos(t), sin(t))*r;
	float d = length(p - dc);
	//return t;
	return d;
}

float circle(vec2 p, vec2 c)
{
	return length(p - c);	
}

float wave(vec2 p)
{
	float y = sin(p.x*2.0)*0.1 + sin(p.x*7.0)*0.05;
	float d = abs(y - p.y);
	return d;
}

void main( void ) {

	vec2 p = (gl_FragCoord.xy / iResolution.xy)*2.0-1.;
	p.x *= iResolution.x / iResolution.y;
	
	//p *= (iMouse.y / iResolution.y) *2.0;
    p *= sin(iGlobalTime)*2.0;
	
	vec3 c = vec3(0.3, 0.2, 0.0);
	float d;
	
	/*
	d = wave(p);	
	c = mix(c, blue, smoothstep(0.11, 0.1, d));
	c = mix(c, orange, smoothstep(0.08, 0.07, d));
	c = mix(c, yellow, smoothstep(0.06, 0.05, d));	
	*/
	
	p = (fract(p*1.0) - vec2(0.5))*vec2(2.0);	
	
	p *= vec2(1.5, 1.5);
	p += vec2(0.8, 0.75);
	
	d = flower(p, vec2(0, 0), 0.5, 8.0, 0.15);
	//c = mix(c, vec3(0.0, 0.0, 0.0), smoothstep(0.53, 0.52, d));	
	c = mix(c, orange, smoothstep(0.5, 0.49, d));
	c = mix(c, blue, smoothstep(0.4, 0.39, d));
	c = mix(c, yellow, smoothstep(0.2, 0.19, d));
	
	//d = dots(p, vec2(0.0, 0.0), 0.4, 8.0, 0.05);
	//c = mix(c, white, d);

	d = dots(p, vec2(0.0, 0.0), 0.6, 16.0);
	c = mix(c, white, smoothstep(0.03, 0.02, d));
	
	
	p -= vec2(1.6, 1.5);

	d = flower2(p, vec2(0, 0), 0.5, 5.0, 0.2);
	c = mix(c, black, smoothstep(0.52, 0.5, d));	
	c = mix(c, blue, smoothstep(0.5, 0.49, d));
	c = mix(c, white, smoothstep(0.15, 0.14, d));
	
	d = circle(p, vec2(0, 0));
	c = mix(c, yellow, smoothstep(0.1, 0.09, d));
	c = mix(c, black, smoothstep(0.05, 0.04, d));
		
	d = dots(p, vec2(0.0, 0.0), 0.6, 10.0);
	c = mix(c, black, smoothstep(0.05, 0.04, d));
	c = mix(c, yellow, smoothstep(0.03, 0.02, d));
	
	//gl_FragColor = vec4( vec3(d), 1);
	gl_FragColor = vec4( c, 1);
}
