// https://www.shadertoy.com/view/XdlSWB

// iChannel0: t7

float x_y = iResolution.x / iResolution.y; 

vec4 filter(sampler2D tex, vec2 uv, float time)
{
	float radius = 0.5;
	vec2 center = vec2(0.5 * x_y,0.5);
	vec2 tc = uv - center;
	float dist = length(tc);
	if (dist < radius)
	{
		float percent = (radius - dist) / radius;
		float theta = percent * percent * (2.0 * sin(time)) * 8.0;
		float s = sin(theta);
		float c = cos(theta);
		tc = vec2(dot(tc, vec2(c, -s)), dot(tc, vec2(s, c)));
	}
	tc += center;
	vec3 color = texture2D(tex, tc).rgb;
	return vec4(color, 1.0);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv.x *= x_y;
	gl_FragColor = filter(iChannel0, uv, iGlobalTime/2.0);
}
