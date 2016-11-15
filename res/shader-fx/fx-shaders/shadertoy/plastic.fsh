// https://www.shadertoy.com/view/MssGRf

// iChannel0: t2
// iChannel1: c0

#define ID_ENV 		0
#define ID_SPHERE 	1
#define ID_PLANE 	2


const float far_dist = 1000.0;

struct ray
{
	vec3 o;
	vec3 d;
};

struct sphere
{
	vec3 c;
	float r;
};

struct plane
{
	vec3 n;
	float d;
};
	
struct intersection
{
	vec3 n;
	vec3 p;
	float t;
};
	
struct surface
{
	int id;
	vec3 p;
	vec3 n;
	vec3 e;
	vec3 a;
};

intersection ray_plane(ray r, plane p)
{
	intersection res;	

	res.t = far_dist;
	
	float t = (p.d - dot(r.o, p.n)) / dot(r.d, p.n);
	
	if( t < 0.0 )
	{
		return res;
	}
	
	res.p = r.o + r.d*t;
	res.t = t;
	res.n = p.n;

	return res;
}
	
intersection ray_sphere(ray r, sphere s)
{
	intersection res;
	res.t = far_dist;
	
	vec3 local_ro = r.o - s.c;
	
	float b = 2.0 * dot(local_ro, r.d);
	float c = dot(local_ro, local_ro) - s.r*s.r;
	float d = b*b - 4.0*c;
	
	if( d < 0.0 )
	{
		return res;
	}

	float t = (-b - sqrt(d)) / 2.0;
	if( t < 0.0 )
	{
		return res;
	}
	
	res.t = t;
	res.p = r.o + r.d * res.t;
	res.n = normalize(res.p - s.c);
	
	return res;	
}

mat3 look_at(vec3 p, vec3 t)
{
	vec3 d = normalize(t-p);
	
	mat3 res;
	
	vec3 up = vec3(0,1,0);
	vec3 right = normalize(cross(up, d));
	up = cross(d, right);
	
	res[0] = right;
	res[1] = up;
	res[2] = d;
	
	return res;
}

surface raytrace(ray r)
{	
	surface res;
	res.id = ID_ENV;
	res.p = r.o + r.d*far_dist;
	res.n = -r.d;
	res.e = r.d;
	res.a = textureCube(iChannel1, r.d).rgb;
	
	float t = far_dist;

	intersection i;

	{
		sphere s;
		s.c = 1.25*vec3(cos(iGlobalTime*2.0),0,sin(iGlobalTime*2.0));
		s.r = 0.25;
		i = ray_sphere(r, s);
		if( i.t < t )
		{
			res.id = ID_SPHERE;
			res.p = i.p;
			res.n = i.n;
			res.e = r.d;
			res.a = vec3(0.1, 1.0, 0.2);
			t = i.t;
		}
	}
	
	{
		sphere s;
		s.c = -1.25*vec3(cos(iGlobalTime*2.0),0,sin(iGlobalTime*2.0));
		s.r = 0.25;
		i = ray_sphere(r, s);
		if( i.t < t )
		{
			res.id = ID_SPHERE;
			res.p = i.p;
			res.n = i.n;
			res.e = r.d;
			res.a = vec3(1.0, 0.2, 0.1);
			t = i.t;
		}
	}
	

	{
		sphere s;
		s.c = vec3(0);
		s.r = 1.0;
		i = ray_sphere(r, s);
		if( i.t < t )
		{
			res.id = ID_SPHERE;
			res.p = i.p;
			res.n = i.n;
			res.e = r.d;
			res.a = vec3(1.0);		
			t = i.t;
		}
	}
	
	{
		plane p;
		p.n = vec3(0,1,0);
		p.d = -1.0;	
		i = ray_plane(r, p);
		if( i.t < t )
		{
			res.id = ID_PLANE;
			res.p = i.p;
			res.n = i.n;
			res.e = r.d;		
			res.a = texture2D(iChannel0, i.p.xz*0.25).rgb;
			t = i.t;
		}
	}
	
	return res;
}

vec3 evaluate(surface surf)
{
	vec3 res = surf.a;
	//	TODO: evaluate materials
	return res;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;	
	
	ray r;
	r.o = vec3(2.0 * cos(15.0+iGlobalTime*0.2), 
			   0.25 + sin(5.0+iGlobalTime*0.05), 
			   4.0 * sin(iGlobalTime*0.1));	
	r.d = vec3(uv*2.0 - 1.0, 1.0);
	r.d.x *= iResolution.x / iResolution.y;
	r.d = look_at(r.o, vec3(0.0)) * normalize(r.d);
			
	int num_bounces = 0;
	const int max_bounces = 3;
	surface surfaces[max_bounces];	
	for(int i=0; i<max_bounces; ++i )
	{
		++num_bounces;
		surface surf = raytrace(r);
		surfaces[i] = surf;
		if( surf.id == ID_ENV )
		{
			break;
		}
		else
		{
			r.d = reflect(surf.e, surf.n);
			r.o = surf.p + r.d*0.001;
			
		}
	}
	
	vec3 res = vec3(0);
	for(int i=max_bounces-1; i>=0; --i)
	{
		float f = pow(max(0., 1.0-max(0.0, dot(-surfaces[i].e, surfaces[i].n))), 4.0);
		res = mix(evaluate(surfaces[i]), res, f);
	}
	
	gl_FragColor = vec4(res, 1.0);
}
