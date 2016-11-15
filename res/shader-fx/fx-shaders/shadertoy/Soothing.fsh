// https://www.shadertoy.com/view/XsBSWW
#define M_PI 3.14159265358979323846
#define FLT_MAX 1000000.0
	
struct line
{
	vec4 line;
	vec3 emissive;
};

bool intersect_line(vec2 ro, vec2 rd, vec4 CD, inout float t)
{
	vec2 A = ro;
	vec2 B = ro + rd;

	vec2 AmC = ro - CD.xy;
	vec2 DmC = CD.zw - CD.xy;
	vec2 BmA = rd;
	
	float denom = (BmA.x*DmC.y)-(BmA.y*DmC.x);
	
	if (denom != 0.0)
	{
		float r = ((AmC.y * DmC.x) - (AmC.x * DmC.y)) / denom;
		float s = ((AmC.y * BmA.x) - (AmC.x * BmA.y)) / denom;
		
		if ((r > 0.0 && r < t) && (s > 0.0 && s < 1.0))
		{
			t = r;
			return true;
		}
	}
	
	return false;
	
}

bool intersect_scene(vec2 ro, vec2 rd, inout float t, out vec3 colour)
{
	float angle = (iGlobalTime * 0.20) * 2.0 * M_PI;
	vec2 dir = vec2(cos(angle), sin(angle));
    	
	bool intersect = false;
	float minDist = t;
	
	vec4 lines[4];
    vec2 normal[4];
	
	vec2 extent0 = dir * 0.1;
    vec2 nrm0 = vec2(dir.y, -dir.x);
	
    vec2 extent1 = vec2(dir.y, -dir.x) * 0.1;
    vec2 nrm1 = dir;
        
	float offset = 0.25;
	
	lines[0] = vec4(vec2(-offset, 0.0) + extent1, vec2(-offset, 0.0) - extent1);
    normal[0] = nrm1;
    
	lines[1] = vec4(vec2( offset, 0.0) + extent1, vec2( offset, 0.0) - extent1);
    normal[1] = -nrm1;
    
	lines[2] = vec4(vec2(0.0, -offset) + extent0, vec2(0.0, -offset) - extent0);
    normal[2] = nrm0;
    
	lines[3] = vec4(vec2(0.0,  offset) + extent0, vec2(0.0,  offset) - extent0);
    normal[3] = -nrm0;
	
	vec3 colours[4];
	float lightComp = 1.0;
	float darkComp = 0.3;
	
	colours[0] = vec3(lightComp, darkComp, darkComp);
	colours[1] = vec3(darkComp, darkComp, lightComp);
	
    colours[2] = vec3(darkComp, lightComp, darkComp);
	colours[3] = vec3(darkComp, lightComp, darkComp);
	
	for (int j = 0; j < 4; j++)
	{
		if (intersect_line(ro, rd, lines[j], minDist))
		{
			colour = colours[j] * abs(dot(-rd, normal[j]));
			intersect = true;
		}	
	}
	
	t = minDist;
	return intersect;
}	

void main(void)
{
	float aspect = iResolution.y / iResolution.x;
	vec2 ro = -1.0 + 2.0 * (gl_FragCoord.xy / iResolution.xy);
	ro.y *= aspect;
	
	vec3 total = vec3(0.0);
	
	for (int i = 0; i < 360; i++)
	{
		float angle = 2.0 * M_PI * (float(i) / 360.0);
		vec2 rd = vec2(cos(angle), sin(angle));
		
		float t = FLT_MAX;
		vec3 colour = vec3(0.0);
		
		if (intersect_scene(ro, rd, t, colour))
		{
			total += colour;
		}
	}
	total /= 360.0;
	
	float exposure = 1.0;
	
	gl_FragColor = vec4(pow(total * exposure, vec3(1.0/2.2)), 1.0);
}
