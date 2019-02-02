// https://www.shadertoy.com/view/XsjGWd

// iChannel0: t14

float t = iGlobalTime;
float noise(vec2 p) { return texture2D(iChannel0, (p+.5)/256., -100.).r; }
vec3 noise3(vec2 p) {
	vec2 F = floor(p), f = fract(p); f*=f*(3.-2.*f);
	return texture2D(iChannel0, (F+f+.5)/256., -100.).rgb; }

struct cellinfo_t {
	vec2 o;
	vec2 p;
	float dist;
	float seed;
};

cellinfo_t cell(vec2 p) {
	vec2 center_origin = floor(p-.5);
	cellinfo_t ret;
	ret.dist = 100000.;
	for(int x = -2; x <= 2; ++x)
		for (int y = -2; y <= 2; ++y)
		{
			vec2 cell_origin = center_origin + vec2(float(x),float(y));
			vec2 local_coord = cell_origin - p + .5;
			vec3 rand = .5*(noise3(cell_origin)+noise3(cell_origin+t+noise3(cell_origin).z));
			local_coord -= (rand.xy-.5)*3.;
			float dist = dot(local_coord,local_coord);
			if (dist < ret.dist) {
				ret.o = cell_origin;
				ret.p = local_coord;
				ret.seed = noise(cell_origin);
				ret.dist = dist;
			}
		}
	return ret;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy * 2. - 1.;
	uv.x *= iResolution.x / iResolution.y;

	cellinfo_t c = cell(uv*12.);
	
	float r = length(c.o);
	
	float chash = floor(r*.2 - t*.7 + .7*(noise(c.o*2.)-.5));
		
	vec3 color = noise3(vec2(chash));
	
	color *= 1.3 - c.dist;

	gl_FragColor = vec4(color,1.);
}
