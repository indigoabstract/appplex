// https://www.shadertoy.com/view/MsXSRr
#define PI 3.1415926
#define MAX_ITER 64
#define MIN_STEP 0.001
#define FOV PI/4.0
#define SAMPLE_RADIUS 0.001
#define SPECULAR 5.0
#define MAX_ITER_SHADOW 32
#define MAX_DIST_SHADOW 10.0
#define MIN_DIST_SHADOW 0.1
#define PENUMBRA 16.0
struct light{
	vec4 c;
	vec4 a;
	vec3 p;	
};
struct ray{
	vec4 c;
	vec3 p;
	vec3 d;
	vec3 n;
	int i;
	light l;
};
struct view{
	vec3 e;
	vec3 l;
	vec3 u;
};
void srand(vec2 p,inout float s);
float rand(inout float s);
float grad(float t);
mat2 rot2d(float a);
mat3 rotx(float a);
mat3 roty(float a);
mat3 rotz(float a);
mat3 rot(vec3 z,float a);
vec4 trace(vec2 p,view v,light l);
float dist(inout ray r);
float sphere(inout ray r,vec3 o);
vec3  normal(ray r);
vec4 ambient(ray r);
vec4 diffuse(ray r,vec3 n);
vec4 specular(ray r,vec3 n);
vec4 shadow(ray r);
float dome(inout ray r);
float smin(float a,float b,float k);
float ifs(inout ray r,vec3 o,float d,int i);
float octahedron(vec3 p,float r);
vec3 texture(vec2 p);
vec3 hash2d(vec2 p);
vec2 simplex(vec2 p);
vec2 unsimplex(vec2 p);
void main(void){
	vec2 p=(gl_FragCoord.xy-iResolution.xy/2.0)/iResolution.y;
	vec2 m=(iMouse.xy-iResolution.xy/2.0)/iResolution.y;
	view v;
	v.l=vec3(0.0,0.0,0.0);
	vec3 vv=vec3(2.0,0.0,0.0);
	vv*=roty((PI*m.y)/4.0-PI/8.0);
	vv*=rotz(2.0*PI*m.x);
	v.e=v.l+vv;
	v.u=vec3(1.0,1.0,1.0);
	light l=light(		
		vec4(0.5),
		vec4(0.5),
		vec3(5.0,5.0,0.0)
	);
	gl_FragColor=trace(p,v,l);
}
vec4 trace(vec2 p,view v,light l){
	ray r;
	r.d=normalize(v.l-v.e);
	vec3 hv=normalize(cross(r.d,v.u));
	vec3 vv=normalize(cross(hv,r.d));
	r.d*=rot(vv,p.x*FOV);
	r.d*=rot(hv,p.y*FOV);	
	r.p=v.e;
	r.i=0;
	r.c=vec4(1.0);
	r.l=l;
	for(int i=0;i<MAX_ITER;i++){
		float d=dist(r);
		if(r.i==1)break;
		d=max(d,MIN_STEP);
		r.p+=r.d*d;
	}
	vec3 n=normal(r);
	return ambient(r)+min(shadow(r),max(diffuse(r,n),specular(r,n)));
}
float dist(inout ray r){
	float d=128.0;
	d=min(d,dome(r));
	return d;
}
vec4 ambient(ray r){
	return r.c*r.l.a;
}
vec4 diffuse(ray r,vec3 n){
	vec3 v=r.l.p-r.p;
	return clamp(r.c*r.l.c*dot(n,normalize(v)),0.0,1.0);
}
vec4 specular(ray r,vec3 n){
	float d=length(r.l.p-r.p);
	vec3 v=normalize(r.l.p-r.p);
	return r.l.c*max(pow(dot(v,reflect(r.d,n)),SPECULAR),0.0);
}
vec4 shadow(ray r){
	r.p-=2.0*MIN_STEP*r.d;
	r.d=normalize(r.l.p-r.p);
	float s=1.0;
	float t=MIN_DIST_SHADOW;
	for(int i=0;i<MAX_ITER_SHADOW;i++){
		ray tmp=r;
		tmp.p+=r.d*t;
		float h=dist(tmp);
		if(h<MIN_STEP)return vec4(0.0);
		s=min(s,PENUMBRA*h/t);
		t+=h;
		if(t>MAX_DIST_SHADOW)break;
	}
	return vec4(s);
}
vec3 normal(ray r){
	float d=dist(r);
	vec3 n=vec3(SAMPLE_RADIUS,0.0,0.0);
	ray r0=r;
	ray r1=r;
	ray r2=r;
	r0.p+=n.xyy;
	r1.p+=n.yxy;
	r2.p+=n.yyx;
	return normalize(vec3(dist(r0)-d,dist(r1)-d,dist(r2)-d));
}
float smin(float a,float b,float k){
	float h=clamp(0.5+0.5*(b-a)/k,0.0,1.0);
    return mix(b,a,h)-k*h*(1.0-h);
}
float dome(inout ray r){
	vec3 tex=texture(r.p.xy);
	float d=dot(r.p+vec3(0.0,0.0,0.5*length(tex)),vec3(0.0,0.0,1.0))+0.5;
	d=smin(d,50.0-length(r.p),10.0);
	if(d<0.0){
		float l=length(r.p);
		r.c=vec4(tex,0.0);
		r.i=1;
	}
	return d;
}
float sphere(inout ray r,vec3 o){
	vec3 p=r.p+o;
	float d=length(p)-0.5;
	if(d<0.0){
		r.i=1;
		r.c=vec4(1.0);
	}
	return d;
}
void srand(vec2 p,inout float s){
	s=sin(dot(p,vec2(423.62431,321.54323)));
}
float rand(inout float s){
	s=fract(s*32322.65432+0.12333);
	return abs(fract(s));
}
vec3 hash2d(vec2 p){
	vec3 c;
	float s;
	srand(p,s);
	c.r=rand(s);
	c.g=rand(s);
	c.b=rand(s);
	return c;
}
float grad(float t){
	return 6.0*pow(t,5.0)-15.0*pow(t,4.0)+10.0*pow(t,3.0);
}
mat2 rot2d(float a){
	float c=cos(a);
	float s=sin(a);
	return mat2(
		c,-s,
		s, c);
}
mat3 rotx(float a){
	float c=cos(a);
	float s=sin(a);
	return mat3(
		1.0,0.0,0.0,
		0.0,  c, -s,
		0.0,  s,  c);
}
mat3 roty(float a){
	float c=cos(a);
	float s=sin(a);
	return mat3(
		c,  0.0,  s,
		0.0,1.0,0.0,
		-s, 0.0,  c);
}
mat3 rotz(float a){
	float c=cos(a);
	float s=sin(a);
	return mat3(
		  c, -s,0.0,
		  s,  c,0.0,
	    0.0,0.0,1.0);
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
float ifs(inout ray r,vec3 o,float d,int i){
	vec3 p=r.p+o;
	float l=0.5;
	for(int i=0;i<16;i++){
		d=min(d,octahedron(p,l));
		p=vec3(p.x+2.5*l,p.y,p.z);
		l*=0.5;
	}	
	if(d<0.0){
		r.i=1;
		r.c=vec4(1.0);
	}
	return d;
}
float octahedron(vec3 p,float r){
	return dot(abs(p),normalize(vec3(1.0,1.0,1.0)))-r;
}
vec3 texture(vec2 p){
	p=simplex(p);
	vec2 p0=floor(p);
	vec2 p1=p0+vec2(1.0,0.0);
	vec2 p2=p0+vec2(1.0,1.0);
	vec2 p3=p0+vec2(0.0,1.0);
	vec2 i=fract(p);
	vec3 r0,r1,r2;
	float d0,d1,d2;	
	if(i.x>i.y){
		r0=hash2d(p0);
		r1=hash2d(p1);
		r2=hash2d(p2);
		d0=max(1.0-distance(unsimplex(p),unsimplex(p0)),0.0);
		d1=max(1.0-distance(unsimplex(p),unsimplex(p1)),0.0);
		d2=max(1.0-distance(unsimplex(p),unsimplex(p2)),0.0);
	}else{
		r0=hash2d(p0);
		r1=hash2d(p2);
		r2=hash2d(p3);
		d0=max(1.0-distance(unsimplex(p),unsimplex(p0)),0.0);
		d1=max(1.0-distance(unsimplex(p),unsimplex(p2)),0.0);
		d2=max(1.0-distance(unsimplex(p),unsimplex(p3)),0.0);
	}
	return d0*r0+d1*r1+d2*r2;
}
vec2 simplex(vec2 p){
	vec2 r;
	r.x=1.1547*p.x;
	r.y=p.y+0.5*r.x;
	return r;
}
vec2 unsimplex(vec2 p){
	vec2 r;
	r.y=p.y-0.5*p.x;
	r.x=p.x/1.1547;
	return r;
}
