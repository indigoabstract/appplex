// https://www.shadertoy.com/view/ldfSzN
#define NUM_ITER 15
#define PI 3.14159265359

float f(vec2 pixel, int i, float mod)
{
	float time = iGlobalTime + 0.3 + mod;
	float v = cos(iGlobalTime + pixel.y + cos(2.0 * iGlobalTime + PI * pixel.x));
	return v;
}

float g(vec2 pixel, int i, float mod)
{
	float time2 = iGlobalTime +  2.0 * 0.3 + mod;
	float v = cos(time2 + pixel.x + cos( 3.0 * (time2 + 0.3) + PI * pixel.y));
	return v;
}

float integrateX(vec2 pixel)
{
	float r = 0.0;
	float dt = 0.3;
	
	const int iter = NUM_ITER;
	
	for(int i = 0; i < iter; ++i)
	{
		r = r + dt * f(pixel, i, 0.3);
	}
	
	return r;
}

float integrateY(vec2 pixel)
{
	float r = 0.0;
	float dt = 0.3;
	
	const int iter = NUM_ITER;
	
	for(int i = 0; i < iter; ++i)
	{
		r = r + dt * g(pixel, i, 0.3);
	}
	
	return r;
}

vec2 density(vec2 pixel)
{
	float x = integrateX(pixel);
	float y = integrateY(pixel);
	return vec2(x, y);
}


vec2 random2f( vec2 seed )
{
	float rnd1 = mod(fract(sin(dot(seed, vec2(14.9898,78.233))) * 43758.5453), 1.0);
	float rnd2 = mod(fract(sin(dot(seed+vec2(rnd1), vec2(14.9898,78.233))) * 43758.5453), 1.0);
	return vec2(rnd1, rnd2);
}

void main(void)
{
	vec2 rand = random2f(vec2(25.0, 10.0));
	
	vec2 uv = (gl_FragCoord.xy + rand) / iResolution.xy;
	
	vec2 d = density(uv);
	
	vec4 color = vec4( (1.0 + sin(d.x * d.y)) * 0.5,
					   (1.0 + cos(d.x + d.y)) * 0.5,
					   (1.0 + sin(d.x + d.y)) * 0.5, 1.0);
	
	gl_FragColor = color;
}
