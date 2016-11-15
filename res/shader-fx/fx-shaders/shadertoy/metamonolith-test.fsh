// https://www.shadertoy.com/view/XtXXRH

float time;
vec2 size;
#define PI 3.141592653589

struct Ray
{
	vec3 org;
	vec3 dir;
};

float hash(float f)
{
    return fract(sin(f*32.34182) * 43758.5453);
}

float hash(vec2 p)
{
    return fract(sin(dot(p.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 grid(vec3 dir, bool vert)
{
    vec2 p = dir.xy / max(0.001, abs(dir.z));
    p *= 3.;
    p.y *= 0.06;
    p.y += time * 20.3;
    vert = hash(floor(p.y/5. + 0.5)) < 0.5 ? vert : !vert;
    p += 0.5;
    float h = hash(floor(p*sign(dir.z)));
    float h2 = hash(floor(p.y/6.));
    float h3 = hash(floor(p.y/20.)+sign(dir.z));
    float band = abs(p.x) < 2. + floor(30.*h3*h3) ? 1. : 0.;
    p = mod(p, vec2(1.));
    p -= 0.5;
    float f = h2 < 0.5 ? smoothstep(0.6, 0.0,length(p))*6. : 2.;
    h = h < h2/1.2 + 0.1 && vert ? 1. : 0.;
    vec3 acc = hsv2rgb(vec3(h2/5.+time/30.,.9,0.9))*h*band*3.*f;
    return acc*pow(abs(dir.z),.5);
}

vec3 background(vec3 dir)
{
    return grid(dir.zxy, true) + grid(dir.yxz, true);
}

vec3 render(Ray ray)
{
    float glow = 0.02;

    return background(ray.dir)*0.5 + glow * vec3(1.9, 2.4, 3.2);
}

Ray createRay(vec3 center, vec3 lookAt, vec3 up, vec2 uv, float fov, float aspect)
{
	Ray ray;
	ray.org = center;
	vec3 dir = normalize(lookAt - center);
	up = normalize(up - dir*dot(dir,up));
	vec3 right = cross(dir, up);
	uv = 2.*uv - vec2(1.);
	fov = fov * 3.1415/180.;
	ray.dir = dir + tan(fov/2.) * right * uv.x + tan(fov/2.) / aspect * up * uv.y;
	ray.dir = normalize(ray.dir);	
	return ray;
}

void mainImage( out vec4 fragColor, in vec2 fragCoord )
{
    size.xy = iResolution.xy;
    time = iGlobalTime * 0.1;
    vec2 p = fragCoord / size;
	vec3 cameraPos = vec3(-8.,2.*sin(time/10.),-4.*sin(time/4.));
	vec3 lookAt = vec3(0.);
	vec3 up = vec3(0.,0.,1.);//mix(vec3(0.,0.,1.), vec3(0.,1.,0.), mod(time, 1.));
	float aspect = size.x/size.y;
	Ray ray = createRay(cameraPos, lookAt, up, p, 90., aspect);
    vec3 col = render(ray);
    col = clamp(col, 0., 1.);
    fragColor = vec4(col, 1.);
}
