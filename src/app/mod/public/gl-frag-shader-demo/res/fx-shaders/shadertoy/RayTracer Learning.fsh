// https://www.shadertoy.com/view/ldXSDj

// iChannel0: t0

struct Ray{
	vec3 origin;
	vec3 direction;
};
    
struct Material{
	float specular;
	float diffuse;
	float ambient;
	float shininess;
    float reflectivity;
    float transparency;
};
    
struct Sphere{
	vec3 position;
	vec3 colour;
	float radius;
    Material mat;
};
   
struct Plane{
	vec3 position;
    vec3 colour;
    vec3 normal;
    Material mat;
};
	
struct PointLight{
	vec3 position;
	vec3 colour;
};


struct Output{
	vec3 origin;
	vec3 normal;
	float dis;
	Material material;
    vec3 colour;
};
    
#define numSphere 6
Sphere sphere[numSphere];

#define numPlane 3
Plane plane[numPlane];

#define numLight 6
PointLight pointlight[numLight];

#define numMaterial 8
Material material[numMaterial];

#define PI 3.14159
vec3 backgroundColour = vec3(0.0, 0.0, 0.7);
vec3 eye;

Ray castRay(vec3 lookat){
	vec2 uv = (gl_FragCoord.xy * 2.1 - iResolution.xy) / iResolution.xx;

	vec3 forward = normalize(lookat - eye);
	vec3 up = vec3(0.0, 1.0, 0.0);
	
	vec3 right = cross(up, forward);
	up = cross(forward, right);
	
	Ray ray;
	
	ray.origin = eye;
	ray.direction = normalize(right * uv.x + up * uv.y + forward);
	
	eye = ray.origin;
	
	return ray;
}


void sphereIntersect(const Sphere sphere, const Ray ray, inout Output o) {
	vec3 d = ray.origin - sphere.position;
	
	float a = dot(ray.direction, ray.direction);
	float b = dot(ray.direction, d);
	float c = dot(d, d) - sphere.radius * sphere.radius;
	
	float g = b*b - a*c;
	
	if(g > 0.0) {
		float dis = (-sqrt(g) - b) / a;
		if(dis > 0.0 && dis < o.dis) {
			o.dis = dis;
			o.origin = ray.origin + ray.direction * dis;
			o.normal = (o.origin - sphere.position) / sphere.radius;
            o.colour = sphere.colour;
			o.material = sphere.mat;
		}
	}
}

void planeIntersect(const Plane plane, const Ray ray, inout Output o){
    float denom = dot(ray.direction, plane.normal);
    if (denom <= 0.0001){
    	return;
    }
    
    vec3 p0l0 = plane.position - ray.origin;
    float t = dot(p0l0, plane.normal) / denom;
    vec3 newOrigin = ray.origin + t * ray.direction;
    float dis = length(t * ray.direction);
    if (dis > 0.0 && dis < o.dis){
    	o.dis = dis;
        o.origin = newOrigin;
        o.normal = plane.normal;
        o.material = plane.mat;
        o.colour = plane.colour;
    }
}

Sphere makeSphere(float offset, Material mat){
    float t = iGlobalTime + offset;
    float x = cos(t);
    float y = sin(t * 1.5);
	float z = sin(t * 2.0) * 3.5;
	return Sphere(vec3(x, y, z),
				  vec3(sin(offset) + 1.0 / 2.0, cos(offset) + 1.0 / 2.0, 0.2),
				  0.5,
                 mat);
}

void makeScene(){
	
    material[0] = Material(0.1, 0.6, 0.1, 0.0, 0.2, 0.0);
    material[1] = Material(0.0, 0.8, 0.0, 0.0, 0.1, 0.0);
    material[2] = Material(0.2, 0.6, 0.5, 0.0, 0.7, 0.0);
    material[3] = Material(0.1, 0.6, 0.1, 1.0, 0.3, 0.0);
    material[4] = Material(0.1, 0.6, 0.7, 0.0, 0.2, 0.0);
    material[5] = Material(0.0, 0.8, 0.0, 0.0, 0.1, 0.0);
    material[6] = Material(0.2, 0.6, 0.5, 0.0, 0.7, 0.0);
    material[7] = Material(0.1, 0.6, 0.1, 1.0, 0.3, 0.0);
    
	for (int i = 0; i < numSphere; i++){
		sphere[i] = makeSphere(float(i), material[i]);
	}
    
    plane[0] = Plane(vec3(0.0, 2.0, 0.0), vec3(1.1, 1.0, 1.0), vec3(0.0,1.0,0.1), material[1]);
    plane[1] = Plane(vec3(0.0, -1.3, 0.0), vec3(1.1, 1.0, 1.1), vec3(0.0,-1.0,0.0), material[1]);
    plane[2] = Plane(vec3(0.0, 0.0, 5.5), vec3(0.0), vec3(1.0,1.0,1.0), material[2]);
	
    float r = 4.0;
	float y = 0.0;
	
	float t0 = -iGlobalTime + PI * 0.0;
	pointlight[0].position = vec3(cos(t0) * r, y, sin(t0) * r);
	pointlight[0].colour = vec3(0.5, 0.0, 0.0);
	float t1 = -iGlobalTime + PI * 0.333333;
	pointlight[1].position = vec3(cos(t1) * r, y, sin(t1) * r);
	pointlight[1].colour = vec3(0.4, 0.4, 0.0);

	float t2 = -iGlobalTime + PI * 0.666666;
	pointlight[2].position = vec3(cos(t2) * r, y, sin(t2) * r);
	pointlight[2].colour = vec3(0.75, 0.75, 0.75);

	float t3 = -iGlobalTime + PI * 1.0;
	pointlight[3].position = vec3(cos(t3) * r, y, sin(t3) * r);
	pointlight[3].colour = vec3(0.0, 0.4, 0.4);

	float t4 = -iGlobalTime + PI * 1.333333;
	pointlight[4].position = vec3(cos(t4) * r, y, sin(t4) * r);
	pointlight[4].colour = vec3(0.0, 0.0, 0.5);

	float t5 = -iGlobalTime + PI * 1.666666;
	pointlight[5].position = vec3(cos(t5) * r, y, sin(t5) * r);
	pointlight[5].colour = vec3(0.4, 0.0, 0.4);
    
	//pointlight[0] = PointLight(vec3(-4.0, 4.0, -8.0), vec3(0.5, 0.5, 0.5));
	//pointlight[1] = PointLight(vec3(4.0, -4.0, 5.0), vec3(0.5, 0.5, 0.5));
    //pointlight[2] = PointLight(vec3(2.0, 5.0, 0.0), vec3(1.0, 0.0, 1.0));

}

vec3 illuminatePointLight(PointLight light, Output o){
    vec3 brightness = vec3(0.0);
    
	vec3 pointToLight = o.origin - light.position;
    for (int j = 0; j < 3; j++){
		brightness[j] += light.colour[j] * o.material.diffuse * dot(normalize(pointToLight), normalize(o.normal));
		brightness[j] += light.colour[j] * o.material.specular * dot(normalize(2.0 * dot(normalize(pointToLight), normalize(o.normal))
												 * normalize(o.normal) - normalize(pointToLight)),
									   eye - o.origin);
    }
    return brightness;
}

vec3 shade(Output o){
    vec3 brightness = vec3(o.material.ambient);
	
	for (int i = 0; i < numLight; i++){
		brightness += illuminatePointLight(pointlight[i], o);
	}
	brightness[0] = clamp(brightness[0], 0.0, 1.0);
    brightness[1] = clamp(brightness[1], 0.0, 1.0);
    brightness[2] = clamp(brightness[2], 0.0, 1.0);
	
    float dis = length(eye - o.origin);
	
	dis -= 10.0;
	dis *= 0.07;
	dis = clamp(dis, 0.0, 1.0);
    brightness *= o.colour;
	return brightness * (1.0 - dis);
}

Output traceStep(in Ray ray){
    Output o  = Output(vec3(0.0), 
                       vec3(0.0, 0.0, 0.1), 
                       1.0e4,
                       Material(
						0.0,
						0.0,
					 	0.0,
						0.0,
						0.0,
                        0.0),
                       vec3(0.0));
                      
	vec3 colour = vec3(0.0);
	for (int i = 0; i < numSphere; i++){
		sphereIntersect(sphere[i], ray, o);
    }
    
    for (int i = 0; i < numPlane; i++){
    	planeIntersect(plane[i], ray, o);
    }
    
    return o;
}

vec3 trace(in Ray ray){
	vec3 colour = vec3(0.0);
    float reflectivity = 1.0;
    Output o;
    
    for (int i = 0; i < 2; i++){
    	o = traceStep(ray);
        
        if (o.dis > 1.0e3) break;
        
        colour += shade(o) * reflectivity;
        
        reflectivity *= o.material.reflectivity;
        
        float l = length(ray.origin - o.origin) + 0.0001;
		colour -= 0.02 / l;

		reflectivity *= o.material.reflectivity;
        
		if(reflectivity < 0.05) {
			break;
		}
		
		ray = Ray(o.origin + o.normal * 0.0001, reflect(normalize(o.origin - ray.origin), o.normal));
    }
    return colour;
}

void main(void)
{
	makeScene();
	//eye = vec3(5.0, 1.0, -2.0);
    eye = vec3(sin(iGlobalTime) * 4.0, cos(iGlobalTime), -4.0);
    Ray ray = castRay(vec3(0.0));
	
	vec3 color = trace(ray);
	gl_FragColor = vec4(color, 1.0);
	
}
