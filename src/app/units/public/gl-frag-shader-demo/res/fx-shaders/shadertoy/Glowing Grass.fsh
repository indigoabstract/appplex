// https://www.shadertoy.com/view/ldsSDM
#define BLADES 10


vec4 grass(vec2 p, float x)
{
	float s = mix(0.9, 1.4, 0.5 + sin(x * 11.0) * 0.2);
	p.x += pow(1.0 + p.y, 2.0) * 0.1 * cos(x * 0.5 + iGlobalTime);
	p.x *= s;
	p.y = (1.0 + p.y) * s - 1.0;
	float m = 1.0 - smoothstep(0.0, clamp(1.0 - p.y * 1.5, 0.1, 0.9) * 1.3 * s, pow(abs(p.x) * 80.0, 2.0) + p.y - 0.1);
	return vec4(vec3(1, 0.2, 0.9), m * smoothstep(-1.0, -0.9, p.y));
}

vec3 backg(vec3 ro, vec3 rd)
{
	vec3 ground = vec3(0.0, 0.0, 0.0);
	return ground;
}

void main()
{
	vec3 ct = vec3(0.0, 2.0, 5.0);
	vec3 cp = vec3(0.0, 0.6, 0.0);
	vec3 cw = normalize(cp - ct);
	vec3 cu = normalize(cross(cw, vec3(0.0, 1.0, 0.0)));
	vec3 cv = normalize(cross(cu, cw));
	
	mat3 rm = mat3(cu, cv, cw);
	
	vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - vec2(1.0);
	vec2 t = uv;
	
	
	vec3 ro = cp, rd = rm * vec3(t, -1.3);
	
	vec3 fcol = backg(ro, rd);
	
	for(int i = 0; i < BLADES; i += 1)
	{
		float z = -(float(BLADES - i) * 0.1 + 1.0);
		vec4 pln = vec4(0.0, 0.0, -1.0, z);
		float t = (pln.w - dot(pln.xyz, ro)) / dot(pln.xyz, rd);
		vec2 tc = ro.xy + rd.xy * t;
		
		tc.x += cos(float(i) * 3.0) * 4.0;
		
		float cell = floor(tc.x);
		
		tc.x = (tc.x - cell) - 0.5;
		
		vec4 c = grass(tc, float(i) + cell * 10.0);
		
		fcol = mix(fcol, c.rgb, step(0.0, t) * c.w);
	}
	
	fcol = pow(fcol * 1.1, vec3(0.8));
	
	
	gl_FragColor.rgb = fcol + vec3(0);
	gl_FragColor.a = 1.0;
}
