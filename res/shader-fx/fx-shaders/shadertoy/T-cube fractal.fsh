// https://www.shadertoy.com/view/MsXGzS
// Public domain

const int   MAX_ITER = 50;
const float MAX_DIST = 10.0;
	
struct Distance
{
	float field;
	int materialId;
	vec4 materialParam;
};
	
struct Hit
{
	vec3 pos;
	Distance dist;
};
	
float distbox(vec3 p, vec3 b)
{
	return length(max(abs(p) - b, 0.0));
}

Distance distfunc(vec3 p)
{
	Distance d;
	d.materialId = 1;
	
	p += 1.0;
	float h = 0.0;
	float c = 1.0;
	float s = 1.0;
	for (int i = 0; i < 6; i++) {
		p = abs(p - s);
		s *= 0.4;
		float di = distbox(p, vec3(s));
		if (di < c) {
			c = di;
			h = float(i) / 6.0;
		}
		
	}
	
	d.materialParam = vec4(h, p);
	d.field = c;
	return d;
}

Hit intersect(in vec3 rayOrigin, in vec3 rayDir)
{
	float total_dist = 0.0;
	vec3 p = rayOrigin;
	Distance d;
	bool hit = false;
	
	for (int i = 0; i < MAX_ITER; i++)
	{
		d = distfunc(p);
		if (d.field < 0.001)
		{
			hit = true;
			//break;
			continue;
		}

		total_dist += d.field;
		if (total_dist > MAX_DIST) {
			//break;
			continue;
		}
		
		p += d.field * rayDir;
	}
	
	Hit h;
	if (hit)
	{
		h.pos = p;
		h.dist.field = total_dist;
		h.dist.materialId = d.materialId;
		h.dist.materialParam = d.materialParam;
	}
	else
	{
		h.dist.materialId = 0;
	}
	return h;
}

vec3 hsv(in float h, in float s, in float v) {
	return mix(vec3(1.0), clamp((abs(fract(h + vec3(3, 2, 1) / 3.0) * 6.0 - 3.0) - 1.0), 0.0 , 1.0), s) * v;
}

vec3 getmaterial(in int materialId, in vec4 materialParam) {
	if (materialId == 1)
	{
		return hsv(materialParam.x, 1.0, 1.0);
	}
	return vec3(0.0);
}

vec3 getnormal(in vec3 p) {
	vec2 e = vec2(0.0, 0.001);
	
	return normalize(vec3(
		distfunc(p + e.yxx).field - distfunc(p - e.yxx).field,
		distfunc(p + e.xyx).field - distfunc(p - e.xyx).field,
		distfunc(p + e.xxy).field - distfunc(p - e.xxy).field));
}

vec3 getlighting(in vec3 pos, in vec3 normal, in vec3 lightDir, in vec3 color)
{
	float b = max(0.0, dot(normal, lightDir));
	return (b * color + pow(b, 32.0));// * softshadow(pos, lightDir, 0.01, 10.0, 16.0);
}

void main()
{
	const vec3 upDirection = vec3(0, 1, 0);
	const vec3 cameraTarget = vec3(0, 0, 0);
	float r = 1.25;
	float theta = iGlobalTime * 0.334543;
	float phi = -iGlobalTime * 0.2345436;
	vec3 cameraOrigin = vec3(sin(theta)*cos(phi), sin(theta)*sin(phi), cos(theta)) * r;
	//vec3 cameraOrigin = vec3(sin(iGlobalTime), sin(iGlobalTime*0.5), cos(iGlobalTime));
	
	vec3 cameraDir = normalize(cameraTarget - cameraOrigin);
	vec3 u = normalize(cross(upDirection, cameraOrigin));
	vec3 v = cross(cameraDir, u);
	vec2 screenPos = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
	screenPos.x *= iResolution.x / iResolution.y;
	vec3 rayDir = normalize(u * screenPos.x + v * screenPos.y + cameraDir);
	
	Hit h = intersect(cameraOrigin, rayDir);
	if (h.dist.materialId != 0)
	{
		vec3 materialColor = getmaterial(h.dist.materialId, h.dist.materialParam);
		vec3 normal = getnormal(h.pos);
		vec3 lightDir = -rayDir;
		vec3 color = getlighting(h.pos, normal, lightDir, materialColor);
	
		gl_FragColor = vec4(color, 1.0);
	}
	else
	{
		gl_FragColor = vec4(0);	
	}
}
