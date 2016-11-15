// https://www.shadertoy.com/view/4sX3Ws
#define STEPS 64

float plane( vec3 p, vec4 n )
{
  return dot(p,n.xyz) + n.w;
}

mat3 my(float beta)
{
	return mat3(cos(beta), 0, sin(beta),
				0, 1, 0,
				-sin(beta), 0, cos(beta));
}

mat3 mz(float beta)
{
	return mat3(cos(beta), -sin(beta), 0,
				sin(beta), cos(beta), 0,
				0, 0, 1);
}

mat3 mx(float beta)
{
	return mat3(1, 0, 0,
				0, cos(beta), sin(beta),
				0, -sin(beta), cos(beta));
}

float sphere(vec3 p)
{	
	return length(p) - 1.0;
}

float grid(float d, vec3 ray, float s, out float col)
{
	vec3 n = normalize(ray);	
	vec3 r = mod(d * n, s);	
	
	if (max(r.x, max(r.y, r.z)) >= s * 0.94)
		col = 1.0;
	else
		col = 0.0;
	
	return d;
}

float map(vec3 ro, vec3 rd, out float col)
{
	float res;
	float val = -1.0;
	vec3 ray = ro + rd;
	for (int j = 0; j < STEPS; j++)
	{
		float t = float(j)/float(STEPS);
		
		float s = sphere(ray);		
		res = min(s,
				  min(
					  plane(ray, normalize(vec4(0.0, 0.5, 0.0, 1.0))),
					  plane(ray, normalize(vec4(0.0, -0.5, 0.0, 1.0)))));		
		if (res < .001)
		{
			float d = length(ray);
			float r;
			val = grid(d, ray, 0.3, r);
			col = r;
			break;
		}
				
		if (length(ray) > 9.0) return -1.0;
		ray += res * rd;
	}
	return val;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float aspect = iResolution.x/iResolution.y;
	vec3 ro = vec3(0.0, 0.0, 3.0);
	vec3 rd = normalize(vec3( (-1.0+2.0*uv) * vec2(aspect, 1.0), -1.0));
	
	#if 1
	float angle, b;
	if (iMouse.z > 0.)
	{
		angle = acos(iMouse.x / iResolution.x);
	}
	else
	{
		angle = mod(iGlobalTime/5.0, 3.1415);
	}
	
	ro *= my(angle);
	rd *= my(angle);

	#endif

	float c;
	float id = map(ro, rd, c);
	
	vec3 col;
	if (id > 0.0)
	{
		if (c == 0.0)
			col = vec3(c); //gaps
		else
			col = vec3(0.0, 0.5*(5.5 - id), 0.0);		
	}
	else
		col = vec3(0.0); //background
	
	gl_FragColor = vec4(col, 1.0);
}
