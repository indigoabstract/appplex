// https://www.shadertoy.com/view/Xsj3WD
#define MAX_DEPTH 10

#define MAT_BACKGROUND    0
#define MAT_GREY_DIFFUSE  1
#define MAT_CHROME        2
#define MAT_WAVY_MIRROR   3

int intersectionMaterial;
float intersectionT;
vec3 intersectionPoint;
vec3 intersectionNormal;

void IntersectSphere(vec3 o, vec3 d, vec3 center, float r, int material)
{
	vec3 to = o - center;
	
    float a = dot(d, d);
    float b = 2. * dot(d, to);
    float c = dot(to, to) - (r * r);
	
	float disc = b * b - 4. * a * c;
	
	if (disc >= 0.)
	{
		float distSqrt = sqrt(disc);
		float q;
		if (b < 0.)
			q = (-b - distSqrt)/2.0;
		else
			q = (-b + distSqrt)/2.0;

		// compute t0 and t1
		float t0 = q / a;
		float t1 = c / q;
		
		float closestT = -1.;
		
		if (t0 > 0.)
			closestT = t0;
		
		if (t1 > 0. && t1 < closestT)
			closestT = t1;
		
		if (closestT > 0. && closestT < intersectionT)
		{
			intersectionMaterial = material;
			intersectionT = closestT;
			intersectionPoint = o + d * intersectionT;
			intersectionNormal = normalize(intersectionPoint - center);
		}
	}
}

void IntersectScene(vec3 o, vec3 d)
{
	IntersectSphere(o, d, vec3(0, 10, 0), 1., MAT_GREY_DIFFUSE);
	IntersectSphere(o, d, vec3(1, 10.2 + .5 * sin(5.*iGlobalTime), 1), .7, MAT_GREY_DIFFUSE);
	IntersectSphere(o, d, vec3(-1, 10.2, 1), .7, MAT_CHROME);
	IntersectSphere(o, d, vec3(0, 10, -400), 398.5, MAT_WAVY_MIRROR);
}

void main(void)
{
	float aspect = iResolution.x / iResolution.y;
	
	vec2 uv = vec2(aspect, 1.) * (gl_FragCoord.xy / iResolution.xy - .5);
	
	float r = fract(10. * sqrt(uv.x * uv.x + uv.y * uv.y));

	vec3 o = vec3(0, 0, 0);
	
	vec3 d;
	
	d = vec3(uv.x, 1, uv.y);
	
	d.xz *= .8;
	
	d = normalize(d);
	
	vec3 color = vec3(0);
	
	bool tracing = true;
	for (int i = 0; i < MAX_DEPTH; ++i)
	{
		intersectionMaterial = MAT_BACKGROUND;
		intersectionT = 1e37;
	
		IntersectScene(o, d);
		
		if (intersectionMaterial == MAT_BACKGROUND)
		{
			float backdrop = dot(d, vec3(0, .7, .7));
			color = backdrop * vec3(.5, .3, .3);
			
			tracing = false;
		}
		else if (intersectionMaterial == MAT_GREY_DIFFUSE)
		{
			color += .1;
			
			float lightDot = dot(intersectionNormal, normalize(vec3(5000, -5000, 10000)));
			
			if (lightDot > 0.)
			{
				color += .9 * lightDot;
			}
		
			tracing = false;
		}
		else if (intersectionMaterial == MAT_CHROME)
		{
			o = intersectionPoint + intersectionNormal * .001;
			d = reflect(d, intersectionNormal);
		}
		else if (intersectionMaterial == MAT_WAVY_MIRROR)
		{
			o = intersectionPoint + intersectionNormal * .001;
			d = reflect(d, normalize(intersectionNormal + .01 * sin(10.*intersectionPoint + 2.*iGlobalTime)));
		}
		
		if (!tracing) break;
	}
	
	gl_FragColor = vec4(color, 1.0);
}
