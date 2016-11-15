// https://www.shadertoy.com/view/Xdl3WM
#define MIN_BANDS 16.
#define MAX_BANDS 32.
#define SPEED 0.7
#define GC_INCREMENT .1
#define GC_STEPS 14.

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float n = ((MAX_BANDS - MIN_BANDS) * .5) * sin(iGlobalTime * SPEED) * 20.;
	n = clamp(n, MIN_BANDS, MAX_BANDS);
	float i = .5 - (floor((.5 - uv.x) * n) + 1.) / n;
	float g = 1. + floor((1. - uv.y) / (1. / (GC_STEPS + 1.))) * GC_INCREMENT;
	i = pow(max(0., i), 1. / g);
	gl_FragColor = vec4(vec3(i), 1.0);
}
