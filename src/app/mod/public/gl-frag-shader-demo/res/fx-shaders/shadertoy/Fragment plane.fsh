// https://www.shadertoy.com/view/MsfXRf
#define ITE_MAX 15

vec2 rot(vec2 p, float a)
{
	return vec2(cos(a) * p.x - sin(a) * p.y, sin(a) * p.x + cos(a) * p.y);
}

vec3 tex(vec2 uv)
{
	vec3 c = vec3(fract(uv.xyy));
	if(mod(uv.x * 2.0, 2.0) < 0.9) return vec3(0);
	if(mod(uv.y * 1.0, 1.0) < 0.9) return vec3(0);
	return c;
}

void main( void ) {

	float M   = iGlobalTime * 0.5;
	float fog = 1.0;
	vec2 uv   = 2.0 * ( gl_FragCoord.xy / iResolution.xy ) - 1.0;
	     uv  *= vec2(iResolution.x / iResolution.y, 1.0);
	     uv   = rot(uv, -iMouse.y * 0.015);
	vec3  c   = vec3(0);
	for(int i = 0 ; i < ITE_MAX; i++) {
		c = tex(vec2(uv.x / abs(uv.y / (float(i) + 1.0)) + M + iMouse.x * 0.015, abs(uv.y)));
		if(length(c) > 0.5) break;
		uv   = uv.yx * 1.3;
		fog *= 0.9;
	}
	gl_FragColor = (1.0 - vec4(c.xyyy * (fog * fog)));
}
