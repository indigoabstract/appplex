// https://www.shadertoy.com/view/MdXXW4
#define M_PI 3.14159265358979323846
#define N 200

void main(void)
{
	vec2 rth = gl_FragCoord.xy / iResolution.xy * 1.5;
	rth.x -= 0.75;
	rth.x *= iResolution.x / iResolution.y;
	float scroll = 27.0 * tan((iMouse.x / iResolution.x * 2.0 - 1.) * 1.25) / tan(1.25);
	rth.x += scroll;
	float thing = 1.0 + abs(rth.x) / M_PI;
	rth = vec2(M_PI / thing, rth.y / thing / thing);
	
	vec2 z0 = (1.0 + rth.y) * vec2(cos(rth.x), sin(rth.x)) / 2.0;
	vec2 z1 = vec2(1.0, 0.0) - z0;
	vec2 c = vec2(z0.x * z1.x - z0.y * z1.y, z0.x * z1.y + z0.y * z1.x);
	vec2 a = c;
	
	float gdec = pow(0.93, 1.0 / (1.0 + abs(scroll) / 10.0));
	float g = pow(abs(scroll) + 1.0, .121212);
	float thresh = 10.0 + 6.0 * sin(iGlobalTime);
	
	for( int i = 0; i < N; ++i ) {
		if (dot(a, a) > thresh)
			continue;
		g *= gdec;
		a = vec2(a.x * a.x - a.y * a.y, 2.0 * a.x * a.y) + c;
	}
	
	if (dot(a, a) > thresh)		
		g *= log(thresh) / log(dot(a, a));
	
	vec2 an = normalize(a);
	vec3 color = vec3(dot(an, vec2(-0.5, sqrt(0.75))),
					  dot(an, vec2(-1.0, 0.0)),
					  dot(an, vec2(-0.5, -sqrt(0.75)))) + 1.0;

	gl_FragColor = vec4(g * color, 1.0);
}
