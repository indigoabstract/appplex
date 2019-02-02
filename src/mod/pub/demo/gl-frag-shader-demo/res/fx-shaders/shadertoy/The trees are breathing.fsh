// https://www.shadertoy.com/view/ldfSW4
#define M_PI 3.14159265358979323846
#define N 53

void main(void)
{
	vec2 rth = gl_FragCoord.xy / iResolution.xy * 2.0 * M_PI;
	rth.y *= iResolution.y / iResolution.x;
	rth.x += (iGlobalTime / 60.0) * 2.0 * M_PI;
	vec2 z0 = (1.0 + rth.y) * vec2(cos(rth.x), sin(rth.x)) / 2.0;
	vec2 z1 = vec2(1.0, 0.0) - z0;
	vec2 c = vec2(z0.x * z1.x - z0.y * z1.y, z0.x * z1.y + z0.y * z1.x);
	vec2 a = c;
	float g = 1.0;
	float thresh = 10.0 + 6.0 * sin(iGlobalTime);
	vec3 color = vec3(1.0, 1.0, 1.0);

	// color += (1.0 + cos (iGlobalTime)) * vec3(0.0, 0.007, 0.01) * clamp(z1.x - z0.x, -2.0, 3.0);
	
	for (int i = 0; i < N; ++i) {
		if (dot(a, a) > thresh) {
			break;
		}
		g *= 0.9;
	    a = vec2(a.x * a.x - a.y * a.y, 2.0 * a.x * a.y) + c;
	}

	gl_FragColor = vec4(g * color, 1.0);
}
