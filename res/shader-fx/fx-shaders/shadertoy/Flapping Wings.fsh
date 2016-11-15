// https://www.shadertoy.com/view/Xdf3Rj
float siny(vec2 uv, float time) {
	float m = uv.x <= 0.0 ? 1.0 : -1.0;
	return sin(m * uv.x * 1.0 + time) - sin(time) * 0.95;
}

float wing(vec2 uv, float time) {
	float m = uv.x <= 0.0 ? 1.0 : -1.0;
	float s = siny(uv, time);
	return clamp(abs(0.005 / (uv.y + s) / (clamp(abs(uv.x), 0.05, 1.0) * m)), 0.0, 1.0);
}

void main(void) {
	vec2 uv = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;

	float time = iGlobalTime * 1.5;

	vec3 color = vec3(0.0);

	// Wings and body
	for (float i=0.0; i<=0.3; i+=0.04) {
		float d = wing(uv, time + i) * 0.25;
		color += vec3(d, d * 0.90, d * 0.0);
	}

	// Slightly red borders
	for (float i=0.0; i<=0.3; i+=0.3) {
		float d = wing(uv, time + i) * 0.25;
		color += vec3(d, d * 0.10, d * 0.0);
	}

	// Trail
	for (float i=-1.25; i<=1.25; i+=0.25) {
		// Skip center trail
		if (i == 0.0)
			continue;

		// Below wing only
		if (uv.y + siny(uv, time) <= 0.0 && uv.y + siny(uv, time + 0.3) <= 0.0) {
			float s = sin(uv.y * 1.0 + time * 0.5) * 0.05;
			float d = abs(0.005 / (uv.x + i + s)) * (1.0 - abs(uv.y) * 0.9);
			d = clamp(d, 0.0, 1.0);
			color += vec3(d * 0.2, d * 0.10, d * 0.0);
		}
	}
	
	gl_FragColor = vec4(color,1.0);
}
