// https://www.shadertoy.com/view/ldlGW4
vec3 rainbow(float h) {
	h = mod(mod(h, 1.0) + 1.0, 1.0);
	float h6 = h * 6.0;
	float r = clamp(h6 - 4.0, 0.0, 1.0) +
		clamp(2.0 - h6, 0.0, 1.0);
	float g = h6 < 2.0
		? clamp(h6, 0.0, 1.0)
		: clamp(4.0 - h6, 0.0, 1.0);
	float b = h6 < 4.0
		? clamp(h6 - 2.0, 0.0, 1.0)
		: clamp(6.0 - h6, 0.0, 1.0);
	return vec3(r, g, b);
}

void main(void)
{
	const float scale = 2.5;
	
	const float startA = 563.0 / 512.0;
	const float startB = 233.0 / 512.0;
	const float startC = 4325.0 / 512.0;
	const float startD = 312556.0 / 512.0;
	
	const float advanceA = 6.34 / 512.0 * 18.2;
	const float advanceB = 4.98 / 512.0 * 18.2;
	const float advanceC = 4.46 / 512.0 * 18.2;
	const float advanceD = 5.72 / 512.0 * 18.2;
	
	vec2 uv = gl_FragCoord.xy * scale / iResolution.xy;
	
	float a = startA + iGlobalTime * advanceA;
	float b = startB + iGlobalTime * advanceB;
	float c = startC + iGlobalTime * advanceC;
	float d = startD + iGlobalTime * advanceD;
	
	float n = sin(a + 3.0 * uv.x) +
		sin(b - 4.0 * uv.x) +
		sin(c + 2.0 * uv.y) +
		sin(d + 5.0 * uv.y);
	
	n = mod(((4.0 + n) / 4.0), 1.0);
	
	gl_FragColor = vec4(rainbow(n), 1.0);
}
