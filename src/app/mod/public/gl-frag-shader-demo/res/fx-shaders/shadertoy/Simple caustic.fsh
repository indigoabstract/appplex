// https://www.shadertoy.com/view/Mdf3DN

// iChannel0: t12

void main(void)
{
	// Texture coordinate
	vec2 uv = gl_FragCoord.xy / iResolution.xy;

	// Lower frequency noise
	float val1 = texture2D(iChannel0, (uv+vec2(iGlobalTime / 100.0, 0.0)) * .3).r;
	float val2 = texture2D(iChannel0, (uv+vec2(0.0, iGlobalTime / 100.0)) * .3).r;

	// Higher frequency noise
	float val3 = texture2D(iChannel0, (uv+vec2(iGlobalTime / 100.0, 0.0)) ).r;
	float val4 = texture2D(iChannel0, (uv+vec2(0.0, iGlobalTime / 100.0)) ).r;

	float val = (val1 * val2) * (.5 + val3 * val4);
	gl_FragColor = vec4(val, val, val, 1.0);
}
