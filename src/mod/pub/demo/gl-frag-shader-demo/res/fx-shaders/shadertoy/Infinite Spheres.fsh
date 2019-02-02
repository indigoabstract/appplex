// https://www.shadertoy.com/view/MsB3Dc
// Infinite spheres with depth of field.
#define PI 3.14159265359
vec3 lp=vec3(10.0,10.0,10.0);
vec4 lc=vec4(0.5);
vec4 la=vec4(0.5);
vec4 mc=vec4(0.0);
float s;
float t;
void srand(vec2 p){
	s=sin(dot(p,vec2(423.62431,321.54323)));
}
float rand(){
	s=fract(s*32322.65432+0.12333);
	return abs(fract(s));
}
mat3 rot_x(float a){
	float c=cos(a);
	float s=sin(a);
	return mat3(1.0,0.0,0.0,0.0,c,-s,0.0,s,c);
}
mat3 rot_y(float a){
	float c=cos(a);
	float s=sin(a);
	return mat3(c,0.0,s,0.0,1.0,0.0,-s,0.0,c);
}
mat3 rot_z(float a){
	float c=cos(a);
	float s=sin(a);
	return mat3(c,-s,0.0,s,c,0.0,0.0,0.0,1.0);
}
mat3 rot(vec3 z,float a){
	float c=cos(a);
	float s=sin(a);
	float ic=1.0-c;
	return mat3(
		ic*z.x*z.x+c,ic*z.x*z.y-z.z*s,ic*z.z*z.x+z.y*s,
		ic*z.x*z.y+z.z*s,ic*z.y*z.y+c,ic*z.y*z.z-z.x*s,
		ic*z.z*z.x-z.y*s,ic*z.y*z.z+z.x*s,ic*z.z*z.z+c);
}
float plane(vec3 p,vec4 n){
	float d=dot(p,n.xyz)+n.w;
	if(d<0.0)mc=vec4(1.0);
	return d;
}
float sphere(vec3 p,float r){
	vec4 c=vec4(1.0);
	vec3 tmp;
	vec2 rp=vec2(floor(p.x),floor(p.y));
	srand(rp);
	c.r=rand()*0.2+0.2*sin(t/4.0+PI/4.0)+0.2;
	c.g=rand()*0.2+0.4*sin(t/3.0+PI/3.0)+0.2;
	c.b=rand()*0.2+0.2*sin(t/2.0)+0.3;
	float x=(fract(p.x)-0.5);
	float y=(fract(p.y)-0.5);
	float tmp2=rand();
	float z=rand()*5.0*(sin(t/2.0+tmp2*PI*2.0)+1.0);
	tmp=vec3(x,y,p.z-z);
	float d=length(tmp)-r;
	if(d<0.0)mc=c;
	return min(d,0.1);
}
float dist(vec3 p){
	float d=100.0;
	d=min(d,plane(p,vec4(0.0,0.0,1.0,0.5)));
	d=min(d,sphere(p,0.5));
	return d;
}
vec3 normal(vec3 p){
	float d=dist(p);
	vec3 n=vec3(0.01,0.0,0.0);
	return normalize(vec3(dist(p+n.xyy)-d,dist(p+n.yxy)-d,dist(p+n.yyx)-d));
}
float trace_light(vec3 p,vec3 dv){
	float d;
	float a=1.0;
	float l=0.1;
	p+=dv*l;
	for(int i=0;i<32;i++){
		d=dist(p);
		if(d<0.01){
			return 0.0;
		}
		a=min(a,2.0*d/l);
		l+=d;
		p+=dv*d;
	}
	return a;
}
vec4 trace(vec3 p,vec3 dv){
	float d;
	float dt=0.0;
	vec3 lv;
	vec4 c;
	mc=vec4(1.0);
	for(int i=0;i<256;i++){
		d=dist(p);
		dt+=d;
		if(d<0.0)break;
		d=max(d,0.01);
		p+=dv*d;
	}
	c=mc;
	vec3 n=normal(p);
	lv=normalize(lp-p);
	vec4 df=clamp(c*lc*dot(n,lv),0.0,1.0);
	vec4 ab=c*la;	
	float sd=trace_light(p-0.01*dv,lv);
	float sp=max(pow(dot(lv,reflect(dv,n)),length(lp-p)),0.0);
	return min(df,vec4(1.0)*sd)+ab+min(sd,sp);
}
void main(void){
	t=iGlobalTime;
	float r=iResolution.x/iResolution.y;
	float h=2.0;
	float fov=0.75;
	vec2 m=vec2((iMouse.x-iResolution.x/2.0)/iResolution.x*r,(iMouse.y-iResolution.y/2.0)/iResolution.y);
	vec2 sp=vec2((gl_FragCoord.x-iResolution.x/2.0)/iResolution.x*r,(gl_FragCoord.y-iResolution.y/2.0)/iResolution.y);
	
	vec3 la=vec3(5.0*sin(t/2.0+PI),5.0*cos(t/3.0+PI),sin(t/4.0)+2.0);
	vec3 ep=vec3(2.0*sin(t),2.0*cos(t),sin(t)*2.0+6.0);
	
	srand(sp);
	vec3 rv;
	float dof=0.2;
	rv.x=(rand()-0.5)*dof;
	rv.y=(rand()-0.5)*dof;
	rv.z=(rand()-0.5)*dof;
	ep+=rv;
	
	
	/*
	vec3 la=vec3(0.0,0.0,h);
	vec3 ep=vec3(0.0,-3.0,0.0);
	ep*=rot_x(PI*m.y/2.0+PI/8.0-PI/4.0);
	ep*=rot_z(2.0*PI*m.x);
	ep.z+=h;
	*/
	
	vec3 uv=vec3(0.0,0.0,1.0);
	vec3 dv=normalize(la-ep);
	vec3 hv=normalize(cross(dv,uv));
	vec3 vv=normalize(cross(hv,dv));
	dv*=rot(vv,fov*sp.x);
	dv*=rot(hv,fov*sp.y);
	gl_FragColor=trace(ep,dv);
}
