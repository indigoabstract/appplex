// https://www.shadertoy.com/view/4slXDH
// srtuss, 2014

#define pi 3.1415926535897932384626433832795

float tri(float x)
{
	return abs(fract(x) * 2.0 - 1.0);
}

float dt(vec2 uv, float t)
{
	vec2 p = mod(uv * 10.0, 2.0) - 1.0;
	float v = 1.0 / (dot(p, p) + 0.01);
	p = mod(uv * 11.0, 2.0) - 1.0;
	v += 0.5 / (dot(p, p) + 0.01);
	return v * (sin(uv.y * 2.0 + t * 8.0) + 1.5);
}

float fun(vec2 uv, float a, float t)
{
	float beat = t * 178.0 / 4.0 / 60.0;
	float e = floor(beat) * 0.1 + 1.0;
	beat = fract(beat) * 16.0;
	float b1 = 1.0 - mod(beat, 10.0) / 10.0;
	float b2 = mod(beat, 8.0) / 8.0;
	b1 = exp(b1 * -1.0) * 0.1;
	b2 = exp(b2 * -4.0);
	e = floor(fract(sin(e * 272.0972) * 10802.5892) * 4.0) + 1.0;
	float l = length(uv);
	float xx = l - 0.5 + sin(mod(l * 0.5 - beat / 16.0, 1.0) * pi * 2.0);
	a += exp(xx * xx * -10.0) * 0.05;
	vec2 pp = vec2(a * e + l * sin(t * 0.4) * 2.0, l);
	pp.y = exp(l * -2.0) * 10.0 + tri(pp.x) + t * 2.0 - b1 * 4.0;
	float v = pp.y;
	v = sin(v) + sin(v * 0.5) + sin(v * 3.0) * 0.2;
	v = fract(v) + b2 * 0.2;
	v += exp(l * -4.5);
	v += dt(pp * vec2(0.5, 1.0), t) * 0.01;
	return v;
}

void main(void)
{
	float t = iGlobalTime;
	vec2 uv = gl_FragCoord.xy / iResolution.xy * 2.0 - 1.0;
	uv.x *= 0.7 * iResolution.x / iResolution.y;
	float an = atan(uv.y, uv.x) / pi;
	float a = 0.02;
	float v =
		fun(uv, an, t + a * -3.) +
		fun(uv, an, t + a * -2.) * 6. +
		fun(uv, an, t + a * -1.) * 15. +
		fun(uv, an, t + a *  0.) * 20. +
		fun(uv, an, t + a *  1.) * 15. +
		fun(uv, an, t + a *  2.) * 6. +
		fun(uv, an, t + a *  3.);
	v /= 64.0;
	vec3 col;
	col = clamp(col, vec3(0.0), vec3(1.0));
	col = pow(vec3(v, v, v), vec3(0.5, 2.0, 1.5) * 8.0) * 3.0;
	col = pow(col, vec3(1.0 / 2.2));
	gl_FragColor = vec4(col, 1.0);
}
