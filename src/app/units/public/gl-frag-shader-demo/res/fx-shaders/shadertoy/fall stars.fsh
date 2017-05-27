// https://www.shadertoy.com/view/lsB3zW
float time = iGlobalTime;
float pi = 3.14159;
float cycle = 2.*pi;

float usin(float x) {return 0.5+0.5*sin(x);}
float osin(float x) {return 0.75+0.25*sin(x);}
float ucos(float x) {return 0.5+0.5*cos(x);}
float ocos(float x) {return 0.75+0.25*cos(x);}

const vec4 nullcol = vec4(0.);
const vec3 nulldir = vec3(0.);

struct shapevals {vec2 pos; float ang; float scale; float line;};
struct rayblob {vec3 pos; float ang; float scale; vec3 col;};
	
vec3 sphere(vec3 p, vec3 r, rayblob s) {
	vec3 inc = p+r;
	if (length(inc-s.pos) < s.scale) {
		float b = 2.*dot(r, p-s.pos);
		float c = dot(p-s.pos, p-s.pos) - s.scale*s.scale;
		float d = b*b-4.*c;
		if (d < 0.) return nulldir;
		float t1 = (-b + sqrt(d))/2.;
		float t2 = (-b - sqrt(d))/2.;
		float t = min(t1,t2);
		return inc-t*r;
	} 
	return nulldir;
}

float surf(vec2 p) {
	float z = usin(p.x*7.+2.*time) + ucos(p.y*4.+osin(time*4.)*2.);	

	return z*.3;
}
	
const vec3 eye = vec3(0.,0.,4.);
const vec3 at = vec3(0.,0.,0.);
const vec3 up = vec3(0.,1.,0.);
const float frustrum = 2.0;
vec4 raysurf(vec2 p, rayblob s) {
	vec3 ray = normalize(at-eye);
	vec3 right = cross(up, ray);

	vec3 pt = normalize(ray*frustrum + p.x*right + p.y*up);
	ray = eye;
	
	vec3 collide;
	vec3 col = vec3(0.);
	
	float dt = .2;
	float z = eye.z;
	for (int i = 0; i < 32; i++) {
		collide = sphere(ray, pt*dt, s);
		if (collide != nulldir) {
			vec3 n = normalize(collide - s.pos);
			pt = -n;
			z = dot(ray, n);
			col += s.col;
			break;
		}
		
		z = surf(ray.xy);
		if (z > ray.z) break;
		ray += pt*dt;
	}
	
	float stripe = ceil(sin(z*(30.*osin(time*4.))));
	vec3 scol = vec3(stripe*.8, stripe*.5, 0.);
	
	col += scol*(z+.3);
	return vec4(col,1.);	
}


float pattern(vec3 ray) {
	float res = 0.;
	float r = length(ray.xy);
	
	return usin(r*cycle*2.-time*10.)+ucos(12.*atan(ray.y/ray.x)+2.*time+r*cycle*10.);
}

vec4 circle(vec2 p, shapevals s, float j) { 
	vec2 pos = s.pos;
	
	vec2 os = p-pos;
	float a = atan(os.y,os.x)+s.ang;
	float d = length(os)-usin(a*j)*s.scale;
	
	if (d<s.scale) {
		return vec4(usin(pos.x), usin(pos.y),pattern(os.xyx),1.0);
	} else if (d-s.line < s.scale) {
		return vec4(nullcol.xyz,1.);
	}return nullcol;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = 2.*uv-1.;
	uv.x *= iResolution.x/iResolution.y;
	
	vec4 col;
	
	shapevals f1 = shapevals(vec2(sin(time),cos(time*4.)), time*4.+sin(time)*2., 0.2,0.05);
	shapevals f2 = shapevals(vec2(cos(time*4.+pi/2.),sin(time)), time*2.-sin(time*2.)*3., 0.25,.05);
	shapevals rain;
	
	rayblob s1 = rayblob(vec3(sin(time), cos(time*2.), .5), 0., 1., vec3(0.,.4,0.));
	
	const int num = 10;
	for (int i = 0; i < num; i++) {
		float j = float(i);
		float fall = usin(sin(j));
		fall = fall*-time*2.;
		rain = shapevals(vec2(mod(j, pi)-pi/2.,mod(j+fall, pi)-pi/2.), time*(0.5+0.5*usin(j))+sin(time+j)*2., 0.1+0.02*usin(cycle*time*4.+j), .05);	
		col = circle(uv, rain, 5.);
		if (col != nullcol) {
			break;
		}
	}
	
	if (col == nullcol) {
		col = circle(uv, f1, 2.);
	}	
	if (col == nullcol) {
		col = circle(uv, f2, 3.);
	}
	if (col == nullcol) {
		col = raysurf(uv, s1);
	}
	
	gl_FragColor = col;
}
