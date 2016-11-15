// https://www.shadertoy.com/view/4dfXD2
vec2 size = vec2(13.0, 11.0);
float scale = 10.0;

float slice(int id) 
{
	// the input image was flipped and inverted
	if (id == 0) return 16245752.0;	if (id == 1) return 2591587.0; if (id == 2) return 6291455.0;
	if (id == 3) return 16706555.0;	if (id == 4) return 16474074.0;	return 8060823.0;
}

float sprite(vec2 p)
{
	p = floor(p);
	if (p.x < 0.0 || p.y < 0.0) return 0.0; // clipping
	if (p.x >= size.x || p.y >= size.y) return 0.0;
	
	float k = p.x + p.y*size.x; // bit ID in sprite
	float n = slice(int(floor(k/24.0))); // get slice
	k = mod(floor(k), 24.0); // bit ID in slice
	
	return 1.0-floor(mod(n/(pow(2.0,k)),2.0)); // get bit of integer (inverted)
}

void main(void)
{
	vec2 p = (gl_FragCoord.xy - 0.5 * (iResolution.xy - size*scale)) / scale; // center and scale
	vec3 col = mix(vec3(0.1), vec3(1.0), sprite(p));
	gl_FragColor = vec4(col, 1.0);
}
