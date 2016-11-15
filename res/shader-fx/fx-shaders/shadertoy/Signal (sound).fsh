// https://www.shadertoy.com/view/4sBSRh
// srtuss, 2014

#define STEPS 80
#define scale -1.0
#define julia vec3(2.2, 0.75, 0.3)

float rnd(float x)
{
    return fract(sin(x * 143.5925) * 98723.8791);
}
float nse(float x)
{
   	float fl = floor(x);
    return mix(rnd(fl), rnd(fl + 1.0), smoothstep(0.0, 1.0, fract(x)));
}
float fbm(float x)
{
    return nse(x) * 0.5 + nse(x * 2.0) * 0.25 + nse(x * 4.0) * 0.125;
}
vec2 rotate(vec2 p, float a)
{
	return vec2(p.x * cos(a) - p.y * sin(a), p.x * sin(a) + p.y * cos(a));
}
float scene(vec3 p)
{
	p *= iGlobalTime;
	vec3 pz = p;
	for(int i = 0; i < 10; i ++)
	{
		vec3 cp = vec3(p.x, -p.y, -p.z);
		p = p + pz / dot(p, p) + julia;
		p = p * scale * 0.3;
	}
	return pow(max(length(p), 0.0), 0.8);
}
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    vec2 pos = uv;
	uv = uv * 2.0 - 1.0;
	uv.x *= iResolution.x / iResolution.y;
    
	vec3 ro = vec3(0.0, 0.0, cos(iGlobalTime * 0.1) * -2.0 - 1.0);
	vec3 rd = normalize(vec3(uv, 1.66));
	
	float t = iGlobalTime * 0.05;
	ro.xz = rotate(ro.xz, cos(t * 2.0));
	rd.xz = rotate(rd.xz, cos(t));
	
	vec3 r = ro;
	float a = 0.0;
	for(int i = 0; i < STEPS; i ++)
	{
		a += scene(r);
		r += rd * 0.05;
	}
	a /= float(STEPS);
	float v = a * 0.15;
	vec3 col = vec3(v);
    col = pow(col, vec3(1.0, 0.6, 0.4) * 6.0) * 5.0;
	col = pow(col, vec3(1.0 / 2.2));
  	col *= 0.1 + 0.9 * pow(16.0 * pos.x * pos.y * (1.0 - pos.x) * (1.0 - pos.y), 0.1);
    col *= fbm(iGlobalTime * 20.0) * 0.4 + 0.7;
	gl_FragColor = vec4(col, 1.0);
}
