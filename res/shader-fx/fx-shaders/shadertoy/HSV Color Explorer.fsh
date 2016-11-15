// https://www.shadertoy.com/view/4sXSDj
vec3 hsv2rgb(in vec3 c) {
    vec3 rgb = clamp(abs(mod(c.x * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 0.0, 1.0);
	rgb = rgb * rgb * (3.0 - 2.0 * rgb);
	return c.z * mix(vec3(1.0), rgb, c.y);
}

void main(void) {
	vec2 mv, uv;
	float v;
	uv = gl_FragCoord.xy / iResolution.xy;
	mv = iMouse.xy / iResolution.xy;
	v = 1.0;
	
	gl_FragColor = vec4(
		hsv2rgb(vec3(uv.x, v, uv.y)),
		1.0
	);
}
