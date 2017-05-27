// https://www.shadertoy.com/view/XdsGDr
#define ITER	120

vec2 map_pixel(in vec2 pix)
{
	float aspect = iResolution.x / iResolution.y;
	return vec2(2.5 * aspect, 2.5) * (pix - vec2(0.5, 0.5));
}

vec3 palval(float x)
{
	vec3 col = vec3(0.0, 0.0, 0.0);
	col.y = smoothstep(0.0, 0.4, x) * 0.8;
	col.z = (smoothstep(0.4, 0.5, x) + smoothstep(0.5, 0.6, x)) * 0.7;
	col.x = smoothstep(0.0, 0.2, x) + smoothstep(0.2, 0.3, x);
	return col;
}

void main() {
	float aspect = iResolution.x / iResolution.y;
	vec2 tc = gl_FragCoord.xy / iResolution.xy;
	
	float t = iGlobalTime * 0.5;

	vec2 c;
	if(iMouse.x < 3.0) {
		c = vec2(cos(t) + sin(t * 3.0) * 0.5, sin(t)) * 0.75;
	} else {
		c = map_pixel(iMouse.xy / iResolution.xy);
	}

	vec2 z = map_pixel(tc);
	
	/*if(length(z - c) < 0.01) {
		gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
		return;
	}*/

	int idx = ITER;
	for(int i=0; i<ITER; i++) {
		float x = (z.x * z.x - z.y * z.y) + c.x;
		float y = (z.y * z.x + z.x * z.y) + c.y;

		if((x * x + y * y) > 4.0) {
			idx = i;
			break;
		}
		z.x = x;
		z.y = y;
	}
	
	vec3 color = palval(mod(float(idx), 50.0) / 50.0);
	if(idx == ITER) {
		color = vec3(0.0, 0.0, 0.0);
	}

	gl_FragColor.xyz = color;
	gl_FragColor.w = 1.0;
}
