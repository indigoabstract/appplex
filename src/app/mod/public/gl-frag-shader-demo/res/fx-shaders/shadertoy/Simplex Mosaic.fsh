// https://www.shadertoy.com/view/Xd23Wt
#define PI 3.14159265359
#define FILTER 1
float s;
void srand(vec2 p){
	s=sin(dot(p,vec2(423.62431,321.54323)));
}
float rand(){
	s=fract(s*32322.65432+0.12333);
	return abs(fract(s));
}
vec4 hash2d(vec2 p){
	srand(p);
	vec4 c;
	c.r=rand();
	c.g=rand();
	c.b=rand();
	return c;
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
vec4 noise2d(vec2 p){
	p=simplex(p);
	vec2 p0=floor(p);
	vec2 p1=p0+vec2(1.0,0.0);
	vec2 p2=p0+vec2(1.0,1.0);
	vec2 p3=p0+vec2(0.0,1.0);
	vec2 i=fract(p);
	vec4 r0,r1,r2;
	float d0,d1,d2;	
	if(i.x>i.y){
		r0=hash2d(p0);
		r1=hash2d(p1);
		r2=hash2d(p2);
		d0=max(1.0-distance(unsimplex(p),unsimplex(p0)),0.0);
		d1=max(1.0-distance(unsimplex(p),unsimplex(p1)),0.0);
		d2=max(1.0-distance(unsimplex(p),unsimplex(p2)),0.0);
		/*
		d0=max(1.0-distance(p,p0),0.0);
		d1=max(1.0-distance(p,p1),0.0);
		d2=max(1.0-distance(p,p2),0.0);
		*/
	}else{
		r0=hash2d(p0);
		r1=hash2d(p2);
		r2=hash2d(p3);
		d0=max(1.0-distance(unsimplex(p),unsimplex(p0)),0.0);
		d1=max(1.0-distance(unsimplex(p),unsimplex(p2)),0.0);
		d2=max(1.0-distance(unsimplex(p),unsimplex(p3)),0.0);
	}
	/*
	float d=d0+d1+d2;
	d0/=d;
	d1/=d;
	d2/=d;
	*/
	return d0*r0+d1*r1+d2*r2;
}
void main(void){
	float r=iResolution.x/iResolution.y;
	vec2 p=vec2((gl_FragCoord.x-iResolution.x/2.0)/iResolution.x*r,(gl_FragCoord.y-iResolution.y/2.0)/iResolution.y);
	vec4 n=vec4(0.0);
	float t=iGlobalTime/PI;
#if FILTER
	vec2 x=vec2(0.5/iResolution.y,0.0);
	x*=rot2d(PI/8.0);
	n+=0.25*noise2d((p+x)*10.0);
	x*=rot2d(PI/4.0);
	n+=0.25*noise2d((p+x)*10.0);
	x*=rot2d(PI/4.0);
	n+=0.25*noise2d((p+x)*10.0);
	x*=rot2d(PI/4.0);
	n+=0.25*noise2d((p+x)*10.0);
#else
	n=noise2d(p*10.0);
#endif
	n.rgb*=rotx(t);
	n.rgb*=roty(t);
	n.rgb*=rotz(t);
	gl_FragColor=vec4(abs(n));
}
