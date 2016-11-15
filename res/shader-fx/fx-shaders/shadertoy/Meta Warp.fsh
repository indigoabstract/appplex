// https://www.shadertoy.com/view/Xsf3zl

// iChannel0: c4

vec3 metaBallPos;
float metaBallRadius = 2.5;

float rand1(float x)
{
    return fract(sin(x) * 4358.5453123);
}

float noise1(float x)
{
	float fl = floor(x);
	float fc = fract(x);
	return mix(rand1(fl), rand1(fl + 1.0), smoothstep(0.0, 1.0, fc)) * 2.0 - 1.0;
}

float fbm1(float x)
{
	return noise1(x) * 0.5 + noise1(x * 2.0) * 0.25;
}


float df(vec3 p)
{
	vec3 m = p - metaBallPos;
	float rsqr = (m.x*m.x + m.y*m.y + m.z*m.z);
	return metaBallRadius / rsqr;
}


void UpdateMetaBall()
{
	float t = iGlobalTime;
	metaBallPos = 8.0*vec3(fbm1(t), fbm1(t), 0.0);
	
}

vec3 RayTrace(vec3 org, vec3 dir)
{
	
	float d=0.0;
	float t = 0.0;
	float dt=0.1;
	vec3  p=org;
	float eps = 0.02;
	for(int i=0; i < 50; i++)
	{
		p = org + t * dir;
		d = df(p);
		if( d > 6.04 )
   			break;
		t += dt;
	}
	p = org + t * dir;
	return p;
}


void main(void)
{
	vec3 camPos = vec3(0.0, 0.0, 5.0);
	
	vec3 rayOrigin = camPos;
	float aspect = iResolution.x / iResolution.y;
	vec2 uv = (gl_FragCoord.xy / iResolution.xy * 2.0 - 1.0) * vec2(aspect, 1.0);
	vec3 rayDir = normalize(vec3(uv, -1.0));

	UpdateMetaBall();
	
	vec3 p = RayTrace(rayOrigin, rayDir);	
	vec3 n = normalize(p - metaBallPos);
	vec3 l = normalize(vec3(1.0, 1.0, 1.0));
	float diffuse = clamp(dot(n, l), 0.0, 1.0);
	vec4 t = vec4(diffuse);
	vec3 r = reflect(rayDir, n);
	vec4 m = textureCube(iChannel0, r).bgra; 	
	gl_FragColor = mix(t, m, 0.75);
	
			
}
