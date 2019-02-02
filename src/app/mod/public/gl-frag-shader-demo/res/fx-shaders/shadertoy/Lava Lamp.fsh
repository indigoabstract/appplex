// https://www.shadertoy.com/view/ldsSD4

// iChannel0: t5

#define DEPTH 10.0
#define STEPS 128
#define PRECISION 0.0001
#define BLUR_R 10.0

#define SHADOWS
#define SHADOWBLUR 8.0

#define RELIEF_Z 0.3
#define RELIEF_DEPTH 0.001


struct Ray {
	vec3 origin;
	vec3 direction;
};

struct Object {
	vec3 position;
	float distance;
	int material;
};

struct Material {
	vec3 color;
	vec3 diffuse;
	vec3 specular;
	float opacity;
};
	

Object TRANSLUCENT;
const int materialCount = 7;
Material materials[materialCount];
float t = iGlobalTime;

vec3 eye = vec3(0,0.4,-1.0);
vec3 light = vec3(0.3,0.5,-0.3);
vec3 lava = vec3(0,0.3,0);

// Thanks iq!
// http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
// http://www.iquilezles.org/www/articles/smin/smin.htm

float sdCappedCylinder(vec3 p,vec2 h){vec2 d=abs(vec2(length(p.xz),p.y))-h;return min(max(d.x,d.y),0.0)+length(max(d,0.0))-0.01;}
float sdCappedCylinderZ(vec3 p,vec2 h){vec2 d=abs(vec2(length(p.xy),p.z))-h;return min(max(d.x,d.y),0.0)+length(max(d,0.0));}
float sdSphere(vec3 p,float r){return length(p)-r;}
float sdFloor(vec3 p,float y){return p.y-y;}
float smin(float a,float b,float k){float res=exp(-k*a)+exp(-k*b);return -log(res)/k;}

mat3 rotX(float a) {float s=sin(a);float c=cos(a);return mat3(1,0,0,0,c,s,0,-s,c);}
mat3 rotY(float a) {float s=sin(a);float c=cos(a);return mat3(c,0,-s,0,1,0,s,0,c);}

void initialize()
{
	// floor
	materials[0] = Material(vec3(0.4,0.2,0.1),vec3(0.2),vec3(0),1.0);
	// glass
	materials[1] = Material(vec3(0.6,0,1),vec3(0.1),vec3(1),0.0);
	// lava
	materials[2] = Material(vec3(1,0,0),vec3(0.5),vec3(2),1.0);
	// cap
	materials[3] = Material(vec3(0.4,0.4,0.6),vec3(0.5),vec3(2.0),1.0);
	// stand
	materials[4] = Material(vec3(0.4,0.4,0.6),vec3(0.5),vec3(2.0),1.0);
	//cable
	materials[5] = Material(vec3(0.1),vec3(0.5),vec3(0.3),1.0);
	// light
	materials[6] = Material(vec3(1),vec3(0),vec3(0),1.0);
}

Material getMaterial(int index)
{
	Material material;
	for (int i=0;i<materialCount;i++)
	{if(index==i){material=materials[i];break;}}
	return material;
}

Object scene(vec3 p, bool shadow)
{
	float dfloor = sdFloor(p,0.0);
	float dglass = sdCappedCylinder(p-vec3(0,0.3,0),vec2(0.1,0.3));
	float dcable = sdCappedCylinderZ((p-vec3(sin(p.z*10.0)*0.1,0.001,-0.5)),vec2(0.015,0.5));
	
	float dcap = max(
		sdCappedCylinder(p-vec3(0,0.62,0),vec2(0.12,0.005)),
		-sdCappedCylinder(p-vec3(0,0.60,0),vec2(0.1,0.005))
	);
	
	dcap = min(dcap, max(
		sdSphere(p*vec3(1,0.8,1)-vec3(0,0.45,0),0.13),
		-sdCappedCylinder(p-vec3(0,0.5,0),vec2(0.13,0.1))
	));
	
	float dstand = max(
		sdCappedCylinder(p-vec3(0,0.05,0),vec2(0.12,0.05)),
		-sdSphere(p+vec3(0.08,0,0.1),0.025)
	);
	
	dstand = max(
		dstand,
		-sdCappedCylinder(p-vec3(0,0.1,0), vec2(0.1,0.025))
	);
	
	float dlight = sdCappedCylinder(p-vec3(0,0.08,0), vec2(0.08,0.01));
	
	float dlava =       sdSphere(p-lava-vec3( 0.03, 0.03-sin(2.7+t*0.7)*0.09, 0.03),0.01);
	dlava = smin(dlava, sdSphere(p-lava-vec3( 0.01, 0.14+cos(1.2-t*1.0)*0.05,-0.03),0.02),32.0);
	dlava = smin(dlava, sdSphere(p-lava-vec3( 0.02,-0.01-sin(0.8+t*0.7)*0.08, 0.00),0.04),32.0);
	dlava = smin(dlava, sdSphere(p-lava-vec3( 0.01, 0.13+cos(0.3+t*0.9)*0.07,-0.03),0.03),32.0);
	dlava = smin(dlava, sdSphere(p-lava-vec3( 0.00,-0.02+sin(0.9-t*0.2)*0.06, 0.01),0.04),32.0);
	dlava = smin(dlava, sdSphere(p-lava-vec3(-0.03, 0.15-cos(2.3+t*0.2)*0.08, 0.03),0.04),32.0);
	
	if (TRANSLUCENT.material == 1) { dglass = 1.0; }
	
	float d = dfloor;
	d = min(d,dglass);
	d = min(d,dlava);
	d = min(d,dstand);
	d = min(d,dcap);
	d = min(d,dcable);
	d = min(d,dlight);
	
	int m;
	
	if(d==dfloor){m=0;}
	else if(d==dglass){m=1;}
	else if(d>=dlava-PRECISION&&d<=dlava+PRECISION){m=2;}
	else if(d==dcap){m=3;}
	else if(d==dstand){m=4;}
	else if(d==dcable){m=5;}
	else if(d==dlight){m=6;}
	
	return Object(p,d,m);
}

Object march(Ray ray)
{
	TRANSLUCENT.material = -1;
	Object obj; float t = 0.0;
	
	for (int i = 0; i < STEPS; i++)
	{
		obj = scene(ray.origin + ray.direction * t,false);
		Material material = getMaterial(obj.material);
		
		if (material.opacity < 1.0 && obj.distance < PRECISION) { TRANSLUCENT = obj; }
		if ((material.opacity==1.0 && obj.distance < PRECISION) || t > DEPTH) { break; }
		
		t += obj.distance;
	}
	
	return obj;
}

float shadowMarch(Ray ray, int m)
{	
	Object obj; float t=0.0, r=1.0;
	
	for (int i=0;i<STEPS;i++)
	{
		obj = scene(ray.origin+ray.direction*t,true);
		if (obj.material == m) { break; }
		if (obj.distance<PRECISION||t>DEPTH){ break; }
		r = min(r,SHADOWBLUR*obj.distance/t); t+=obj.distance;
	}

	return r;
}

Ray lookAt(vec3 origin, vec3 target)
{
	vec2 uv = (2.0 * gl_FragCoord.xy - iResolution.xy) / iResolution.xx;
	vec3 dir = normalize(target-origin), up = vec3(0,1,0), right = cross(up, dir);
	return Ray(origin, normalize(vec3(uv.x*right + uv.y*cross(dir, right) + dir)));
}

vec3 getNormal(vec3 p)
{
	vec2 e = vec2(PRECISION,0);
	vec3 n = normalize(vec3(
		scene(p+e.xyy,false).distance - scene(p-e.xyy,false).distance,
		scene(p+e.yxy,false).distance - scene(p-e.yxy,false).distance,
		scene(p+e.yyx,false).distance - scene(p-e.yyx,false).distance
	));
	
	return n;
}

vec3 relief(vec2 p)
{
	vec2 e = vec2(RELIEF_DEPTH,0);
	vec3 n = normalize(vec3(
		length(texture2D(iChannel0,p+e.xy).rgb) - length(texture2D(iChannel0,p-e.xy).rgb),
		length(texture2D(iChannel0,p+e.yx).rgb) - length(texture2D(iChannel0,p-e.yx).rgb),
		RELIEF_Z
	));
	
	return n;
}

vec3 processColor(Object obj)
{
	Material material = getMaterial(obj.material);
	if (TRANSLUCENT.material > 0 && obj.material == 0) { obj.position.xz -= 0.1; }
	if (obj.material == 1) { obj.position.x += 0.5; }
	if (obj.material == 2) { light = vec3(0); }
	
	vec3 col = material.color;
	vec3 n = getNormal(obj.position);
	vec3 l = normalize(light-obj.position);
	
	if (obj.material == 0) {
		n = relief(obj.position.xz);
		col = texture2D(iChannel0,obj.position.xz).rgb;
		col *= 0.5+0.5*clamp(8.0*(length(obj.position.xz)-0.1),0.0,1.0);
	} else {
		col *= 0.5+0.5*smoothstep(0.0,0.025,obj.position.y);
	}
	
	float distance = min(1.0,0.5/length(light-obj.position));
	float diffuse = max(dot(n,reflect(-l,n)),0.0);
	float specular = pow(diffuse, 100.0);
	
	col += vec3(
		diffuse*material.diffuse+
		specular*material.specular
	);
	
	if (obj.material != 5) { col *= distance; }
	
	#ifdef SHADOWS
		Ray ray = Ray(light,normalize(obj.position));
		col *= max(shadowMarch(ray,obj.material),0.5);
	#endif
	
	return col;
}


void main(void)
{
	initialize();

	eye *= rotY(iMouse.x/iResolution.x);
	eye *= rotX(-iMouse.y/iResolution.y);
	
	//light *= rotY(t);
	
	Ray ray = lookAt(eye,vec3(0,0.25,0));
	Object obj = march(ray);
	
	vec3 col = vec3(0);
	
	if (obj.distance < PRECISION || TRANSLUCENT.material > 0)
	{
		col = processColor(obj);
		if (TRANSLUCENT.material > 0) { col += processColor(TRANSLUCENT); }
	}
	
	gl_FragColor = vec4(col*vec3(1,0.8,0.6),1.0);
}
