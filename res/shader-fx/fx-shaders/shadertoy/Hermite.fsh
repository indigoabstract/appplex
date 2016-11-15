// https://www.shadertoy.com/view/4sS3Wc

// iChannel0: t4

// Playing with Hermite interpolation. I was looking for a way to distort a 2D space while keeping monotonicity. This is a Hermite based one.
/*

This shader distorts a 2D space while maintaining
monotonicity, by using Hermite interpolation with
factors up to 3.

Thanks to Reedbeta for the suggestion.

--
Zavie

*/

float h00(float x) { return 2.*x*x*x - 3.*x*x + 1.; }
float h10(float x) { return x*x*x - 2.*x*x + x; }
float h01(float x) { return 3.*x*x - 2.*x*x*x; }
float h11(float x) { return x*x*x - x*x; }

float Hermite(float p0, float p1, float m0, float m1, float x)
{
	return p0*h00(x) + m0*h10(x) + p1*h01(x) + m1*h11(x);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;

	float a = sin(iGlobalTime * 1.0)*0.5 + 0.5;
	float b = sin(iGlobalTime * 1.5)*0.5 + 0.5;
	float c = sin(iGlobalTime * 2.0)*0.5 + 0.5;
	float d = sin(iGlobalTime * 2.5)*0.5 + 0.5;
	
	float y0 = mix(a, b, uv.x);
	float y1 = mix(c, d, uv.x);
	float x0 = mix(a, c, uv.y);
	float x1 = mix(b, d, uv.y);

	uv.x = Hermite(0., 1., 3.*x0, 3.*x1, uv.x);
	uv.y = Hermite(0., 1., 3.*y0, 3.*y1, uv.y);

	
	vec3 color = texture2D(iChannel0, vec2(uv.x, 1. - uv.y)).xyz;
	vec2 grid = fract(0.05*iResolution.xy*uv);
	gl_FragColor = vec4(mix(0.8, 1., 0.5*(grid.x + grid.y))*color,1.0);
}
