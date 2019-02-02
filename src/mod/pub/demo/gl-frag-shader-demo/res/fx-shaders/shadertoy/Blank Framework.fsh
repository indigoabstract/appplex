// https://www.shadertoy.com/view/ld2GDc
// Blank framework for the raymarching neophyte.
#define PI 3.14159265359
#define MAX_ITER 128
#define MAX_ITER_INTERNAL 0
#define MAX_ITER_SHADOW 32
#define MAX_DIST_SHADOW 10.0
#define MIN_DIST_SHADOW 0.1
#define MAX_DIST 1000.0
#define MIN_DIST 0.01
#define FOV 0.75
#define SPECULAR 5.0
#define SAMPLE_RADIUS 0.001
#define PENUMBRA 2.0
struct ray{
	vec4 c;
	vec3 p;
	vec3 d;
	vec3 n;
	float t;
	int i;
};
struct light{
	vec4 c;
	vec3 p;
};
light l=light(
	vec4(0.5),
	vec3(0.0,-5.0,5.0)
);
vec4 a=vec4(0.5);
float s;
void srand(vec2 p){
	s=sin(dot(p,vec2(423.62431,321.54323)));
}
float rand(){
	s=fract(s*32322.65432+0.12333);
	return abs(fract(s));
}
mat3 rotx(float a){
	float c=cos(a);
	float s=sin(a);
	return mat3(1.0,0.0,0.0,0.0,c,-s,0.0,s,c);
}
mat3 roty(float a){
	float c=cos(a);
	float s=sin(a);
	return mat3(c,0.0,s,0.0,1.0,0.0,-s,0.0,c);
}
mat3 rotz(float a){
	float c=cos(a);
	float s=sin(a);
	return mat3(c,-s,0.0,s,c,0.0,0.0,0.0,1.0);
}
mat3 rot(vec3 z,float a){
	float c=cos(a);
	float s=sin(a);
	float b=1.0-c;
	return mat3(
		b*z.x*z.x+c,b*z.x*z.y-z.z*s,b*z.z*z.x+z.y*s,
		b*z.x*z.y+z.z*s,b*z.y*z.y+c,b*z.y*z.z-z.x*s,
		b*z.z*z.x-z.y*s,b*z.y*z.z+z.x*s,b*z.z*z.z+c);
}
mat2 rot2d(float a){
	float c=cos(a);
	float s=sin(a);
	return mat2(
		c,-s,
		s, c);
}
float expstep(float x,float k,float n){
	return exp(-k*pow(x,n));
}
float smin(float a,float b,float k){
	float h=clamp(0.5+0.5*(b-a)/k,0.0,1.0);
    return mix(b,a,h)-k*h*(1.0-h);
}
float dome(inout ray r){
	float d=dot(r.p,vec3(0.0,0.0,1.0))+0.5;
	d=smin(d,50.0-length(r.p),10.0);
	if(d<0.0){
		float l=length(r.p);
		r.c=vec4(1.0);
		r.i=MAX_ITER_INTERNAL+1;
	}
	return d;
}
float sphere(inout ray r){
	float d=length(r.p)-0.5;	
	if(d<0.0){
		r.c=vec4(1.0);
		r.i=MAX_ITER_INTERNAL+1;
	}
	return d;
}
float dist(inout ray r){
	float d=MAX_DIST;
	d=min(d,dome(r));
	d=min(d,sphere(r));
	return d;
}
void normal(inout ray r){
	float d=dist(r);
	vec3 n=vec3(SAMPLE_RADIUS,0.0,0.0);
	ray r0=r;
	ray r1=r;
	ray r2=r;
	r0.p+=n.xyy;
	r1.p+=n.yxy;
	r2.p+=n.yyx;
	r.n=normalize(vec3(dist(r0)-d,dist(r1)-d,dist(r2)-d));
}
vec4 ambient(ray r){
	return r.c*a;
}
vec4 diffuse(ray r){
	vec3 v=l.p-r.p;
	return clamp(r.c*l.c*dot(r.n,normalize(v)),0.0,1.0);
}
vec4 specular(ray r){
	float d=length(l.p-r.p);
	vec3 v=normalize(l.p-r.p);
	return l.c*max(pow(dot(v,reflect(r.d,r.n)),SPECULAR),0.0);
}
vec4 shadow(ray r){
	float s=1.0;
	float t=MIN_DIST_SHADOW;
	for(int i=0;i<MAX_ITER_SHADOW;i++){
		ray tmp=r;
		tmp.p+=r.d*t;
		float h=dist(tmp);
		if(h<MIN_DIST)return vec4(0.0);
		s=min(s,PENUMBRA*h/t);
		t+=h;
		if(t>MAX_DIST_SHADOW)break;
	}
	return vec4(1.0)*s;
}
vec4 trace(inout ray r){
	r.c=vec4(1.0);
	for(int i=0;i<MAX_ITER;i++){
		float d=dist(r);
		if(r.i>MAX_ITER_INTERNAL)break;
		r.p+=r.d*max(d,MIN_DIST);
	}
	normal(r);
	ray tmp=r;
	tmp.d=normalize(l.p-r.p);
	tmp.p-=2.0*MIN_DIST*r.d;	
	return ambient(r)+min(max(diffuse(r),specular(r)),shadow(tmp));
}
void main(void){
	float t=iGlobalTime;
	float r=iResolution.x/iResolution.y;
	vec2 m=vec2(
		(iMouse.x-iResolution.x/2.0)/iResolution.x*r,
		(iMouse.y-iResolution.y/2.0)/iResolution.y);
	vec2 s=vec2(
		(gl_FragCoord.x-iResolution.x/2.0)/iResolution.x*r,
		(gl_FragCoord.y-iResolution.y/2.0)/iResolution.y);	
	vec3 l=vec3(0.0,0.0,0.0);
	vec3 tmp=vec3(2.0,0.0,0.0);
	tmp*=roty((PI*m.y)/4.0-PI/8.0);
	tmp*=rotz(2.0*PI*m.x);
	vec3 e=l+tmp;
	vec3 u=vec3(0.0,0.0,1.0);
	vec3 d=normalize(l-e);
	vec3 h=normalize(cross(d,u));
	vec3 v=normalize(cross(h,d));
	float f=0.75;
	d*=rot(v,FOV*s.x);
	d*=rot(h,FOV*s.y);
	ray a=ray(vec4(0.0),e,d,vec3(0.0),t,0);
	gl_FragColor=trace(a);
}
