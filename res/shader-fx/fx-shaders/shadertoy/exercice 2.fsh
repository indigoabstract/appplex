// https://www.shadertoy.com/view/ldXSDH

// iChannel0: t0
// iChannel1: t0
// iChannel2: c3

// Mon shader

struct sphere_t{
	vec3 position;
	float radius;
	vec3 normal;
	vec3 color;
	float ka;
};
	
struct surface_t{
	vec3 position;
	float identify;
	vec3 normal;
	vec2 uv;
	vec3 color;
	vec3 collisionpoint;
};
	
struct ray_t{
	vec3 origin;
	vec3 direction;
	
};
	
struct light_t{
	vec3 direction;
	float intensity;
	vec3 position;
};	
	
struct plane_t{
	float a, b, c, d;
	vec3 color;
	float ka;
};

vec3 mult(float t, vec3 v){
	return vec3(v.x*t, v.y*t, v.z*t);
}

float magnitude(vec3 v){
	return sqrt(v.x*v.x + v.y*v.y + v.z*v.z);
}
	
float fAttenuation(float d){
	float attenuation = 1./(0.1 + 5e-5*d + 7e-5*d*d);
	return min(attenuation, 1.);
}

vec3 reflectedVec(vec3 normal, vec3 ray){
	return normalize(mult(2.* dot(normal, -ray), normal) + ray);
}

float intersectSphere( in ray_t ray, inout sphere_t sphere){
	float c=pow(ray.origin.x-sphere.position.x,2.0)+pow(ray.origin.y-sphere.position.y,2.0)+pow(ray.origin.z-sphere.position.z,2.0)-pow(sphere.radius,2.0);
	float b=2.0*(ray.direction.x * (ray.origin.x-sphere.position.x) + ray.direction.y * (ray.origin.y-sphere.position.y) + ray.direction.z*(ray.origin.z-sphere.position.z));
	float a = pow(ray.direction.x,2.0) + pow(ray.direction.y,2.0) + pow(ray.direction.z,2.0);
	float delta = pow(b,2.0) -4.0 * a * c;
	
	float t =min((-b-sqrt(delta))/(2.0*a), (-b+sqrt(delta))/(2.0*a));
	if(delta>=0.0){
		sphere.normal= vec3( ray.origin.x + t * ray.direction.x - sphere.position.x, ray.origin.y + t* ray.direction.y - sphere.position.y, ray.origin.z + t* ray.direction.z - sphere.position.z)  ;
		return t;
	}
	else{
		 sphere.normal=vec3(0.0, 0.0, 0.0);
		 return 1e5;
	}
}

vec3 collisionPoint(ray_t ray, float t){
	return vec3(ray.origin.x + ray.direction.x*t, ray.origin.y + ray.direction.y*t, ray.origin.z + ray.direction.z*t);
}

float intersectPlane(const in ray_t ray, const in plane_t plane){
	float numerateur= -(ray.origin.x * plane.a + ray.origin.y * plane.b + ray.origin.z * plane.c + plane.d);
	float denominateur= plane.a * ray.direction.x + plane.b * ray.direction.y + plane.c * ray.direction.z;
	float t= numerateur/denominateur;
	if(t<0.0)
		return 1e5;
	else 
		return t;
}

vec3 illumination(light_t directional_light, ray_t ray, in surface_t surface, in light_t light, sphere_t sphere, plane_t plane, vec3 color, int type){
	vec3  firstColor, L, R, L_directional;
	L= light.position - surface.collisionpoint;
	float attenuation=fAttenuation(magnitude(L));
	L = normalize(L);
	R = reflectedVec(surface.normal, ray.direction);
	float kAttenuationCubeMap = pow(1.0-dot(surface.normal, -ray.direction),2.);
	vec3 diffuse;
	float speculaire;
	
	vec3 cubemapColor = textureCube(iChannel2,R).xyz;
	
	ray_t lumRay, transRay, refRay;
	lumRay.direction = L;
	lumRay.origin = surface.collisionpoint;
	
	
	
	L_directional= normalize(directional_light.position - surface.collisionpoint);
	float sourceDirectionnal = pow(dot(-L_directional, normalize(directional_light.direction)), 6.);
	vec3 ambiante;
	float intensiteAmbiante = 1.5;
	
	if(type==0){
		
		color = texture2D( iChannel1, surface.uv ).xyz;
		
		ambiante.x = color.x * plane.ka * intensiteAmbiante;
		ambiante.y = color.y * plane.ka * intensiteAmbiante;
		ambiante.z = color.z * plane.ka * intensiteAmbiante;
		
		if(intersectSphere(lumRay, sphere)==1e5){
			diffuse.x = color.x*dot(surface.normal, L);
			diffuse.y = color.y*dot(surface.normal, L);
			diffuse.z = color.z*dot(surface.normal, L);
			
			speculaire =  pow(dot(L, R), 10.)*0.4;

		}		
	}
	if(type==1){
		
		color = texture2D( iChannel0, surface.uv ).xyz;
		ambiante.x = color.x * plane.ka * intensiteAmbiante;
		ambiante.y = color.y * plane.ka * intensiteAmbiante;
		ambiante.z = color.z * plane.ka * intensiteAmbiante;
		
		if(intersectPlane(lumRay, plane)==1e5){
			diffuse.x = color.x*dot(surface.normal, L);
			diffuse.y = color.y*dot(surface.normal, L);
			diffuse.z = color.z*dot(surface.normal, L);
			
			speculaire =  pow(dot(L, R), 10.)*0.4;
			
		}
	}
	
	firstColor.x = kAttenuationCubeMap*cubemapColor.x * ambiante.x + attenuation * light.intensity* sourceDirectionnal * (diffuse.x + speculaire);
	firstColor.y = kAttenuationCubeMap*cubemapColor.y * ambiante.y + attenuation * light.intensity* sourceDirectionnal * (diffuse.y + speculaire);
	firstColor.z = kAttenuationCubeMap*cubemapColor.z * ambiante.z + attenuation * light.intensity* sourceDirectionnal * (diffuse.z + speculaire);

	return firstColor;
}


vec2 setUV(vec3 p, float radius){
	vec2 uv;
	float x,y,z;
	x = p.z;
	y = p.x;
	z = p.y;
	radius = sqrt(x*x + y*y +z*z);
	uv.y = acos(z/radius)/3.14;
	
	if(y>=0.)
		uv.x = acos(x/sqrt(x * x + y * y))/(2.*3.14);
	else
		uv.x = (2.*3.14 - acos(x/sqrt(x * x + y * y)))/(2.*3.14);
	
	return uv;
}

vec3 sendRay(inout surface_t surface, ray_t ray, inout sphere_t sphere, plane_t plane, light_t light, light_t directional_light){
	float max_distance =100.;
	
	
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec3 normal;
	
	float distance_from_origin_plane= intersectPlane(ray, plane);
	float distance_from_origin_sphere = intersectSphere(ray, sphere);

	if( distance_from_origin_sphere < distance_from_origin_plane )
	{
			surface.identify=1.0;
			surface.collisionpoint = collisionPoint(ray, distance_from_origin_sphere);
			surface.normal=normalize(surface.collisionpoint - sphere.position);
			surface.uv = setUV(surface.collisionpoint , sphere.radius);
			
			vec3 sphere_color = illumination(directional_light, ray, surface,light, sphere, plane, sphere.color, 1);
			return sphere_color;
			
	}
	else if( distance_from_origin_plane < max_distance )
	{
			surface.identify=0.0;			
			normal=vec3(plane.a, plane.b, plane.c);			
			surface.normal=normalize(normal);
			surface.collisionpoint = collisionPoint(ray, distance_from_origin_plane);
			surface.uv.x = surface.collisionpoint.x;
			surface.uv.y = -surface.collisionpoint.z;
			
			vec3 plane_color = illumination(directional_light, ray, surface,light, sphere, plane, plane.color, 0);
			surface.color = plane.color;
			return plane_color;
	}
	else
	{
		surface.identify = -1.;
		vec3 L = reflectedVec(surface.normal, ray.direction);
		return textureCube(iChannel2, L).xyz;
	}
}

void main(void){
	float max_distance=100.0;
	vec2 screen_coordonate = gl_FragCoord.xy / iResolution.xy;
	vec2 screen_coordonate_centered = 2.0 * screen_coordonate -1.0;
	surface_t surface;
		
	ray_t primary_ray;
	primary_ray.origin =vec3(0.0, 0.0, mix( 3.0, 5.0, 0.5 *sin(iGlobalTime)+0.5));
	primary_ray.direction =normalize(vec3(screen_coordonate_centered.x, screen_coordonate_centered.y * iResolution.y/iResolution.x, -1));
	
	light_t light;
	light.intensity=2.;
	light.position = vec3(0., 10., 0.);
	
	light_t directional_light;
	directional_light.intensity=.9;
	directional_light.direction = vec3(0., -1., 0.);
	directional_light.position = vec3(0., 10., 0.);
	
	sphere_t sphere;
	sphere.position =vec3(0.0,2.0,-5.0);
	sphere.radius=2.0;
	sphere.color = vec3(0., 0.9, 0.);
	sphere.ka =0.6;
	
	plane_t plane;
	plane.a=0.0;
	plane.b=0.9;
	plane.c=0.04;
	plane.d=0.8;
	plane.color = vec3(0.9, 0., 0.);
	plane.ka = 0.5;	

	gl_FragColor = vec4(sendRay(surface, primary_ray, sphere, plane, light,  directional_light), 1);

}
