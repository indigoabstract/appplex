// https://www.shadertoy.com/view/XdXXzn
float gRaymarchEpsilon = 0.001;
//float gMusicSamplert
vec2 GetUV();
float plane(vec3 p, vec3 n)
{
	return dot(p,n);
}
float sphere(vec3 p, float radius)
{
	float modval = 2. * radius + radius;
	p.xz = mod(p.xz, modval) - modval / 2.;
	
	return length(p) - radius;
}

// distance estimator
float DE(vec3 p)
{
	float s0 = sphere(p, 0.25);
	float p0 = plane(p, normalize(vec3(0,1,0.1)));
	return min(s0, p0);
	return s0;
}

void raymarch( vec3 ray_start, vec3 ray_dir, out float depth, out float dist)
{
   vec3 point = ray_start;
   
	dist = 1.0;
	depth = 0.0;
	for (int i = 0; i < 100; ++i)
	{
		if (dist < gRaymarchEpsilon) continue;
		vec3 point = ray_start + ray_dir * depth;
		dist = DE(point);
		depth += dist;
	}
}

vec2 GetUV()
{
	float aspect = iResolution.x / iResolution.y;
	vec2 uv = gl_FragCoord.xy;
	uv /= iResolution.xy;
	uv *= 2.0;
	uv -= 1.0;
	uv.x *= aspect;
	
	return uv;
}

vec3 GetNormal(vec3 point)
{
	float epsilon= 0.1;
	vec3 dx = vec3(epsilon,0,0);
	vec3 dy = vec3(0,epsilon,0);
	vec3 dz = vec3(0,0,epsilon);
	return normalize(vec3(
		DE(point + dx)- DE(point - dx),
		DE(point + dy)- DE(point - dy),
		DE(point + dz)- DE(point - dz)));
}

void main( void ) {
    vec2 uv = GetUV();
		
	float camZ = 0.115 * sin(iGlobalTime) + 0.25 + 1. + 0.1;
	float camY = 0.115 * sin(iGlobalTime) + 0.45;
	vec3 camPos = vec3(0,camY, camZ);
	vec3 camDir = normalize(-camPos);
	
    vec3 rayStart = camPos;
    vec3 rayDir = normalize(vec3(uv,0) - rayStart);
    
    vec3 color = vec3(0);
	float depth;
	float dist;
    raymarch(rayStart, rayDir, depth, dist);
    if (dist < gRaymarchEpsilon)
	{
		vec3 n = GetNormal(rayStart + rayDir * depth);
		vec3 diffuseCol = vec3((uv + 1.) / 2., 0);
		vec3 specularCol = vec3(0.7,0.7,0.7);
		float shinyness = 128.;
		
		vec3 l = normalize(vec3(.5,1,1));
		vec3 dPara = n * dot(n, camDir);
		vec3 dPerp = camDir - dPara;
		vec3 r = normalize(-dPara + dPerp);
		color += diffuseCol * max(dot(n,l), 0.0);
		color += specularCol * pow(max(dot(n, r), 0.0), shinyness);
		
	}
	else
	{
		//color.xy += uv;
	}
	gl_FragColor = vec4(color,1);
}
