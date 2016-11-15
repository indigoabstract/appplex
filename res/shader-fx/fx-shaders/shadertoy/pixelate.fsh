// https://www.shadertoy.com/view/4sl3zr

// iChannel0: t4

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 divs = vec2(iResolution.x * 20.0 / iResolution.y, 20.0);
	uv = floor(uv * divs)/ divs;
	gl_FragColor = texture2D(iChannel0, uv);
}
