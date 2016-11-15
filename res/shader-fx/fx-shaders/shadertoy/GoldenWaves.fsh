// https://www.shadertoy.com/view/4slSDr
const float PI=3.14159;

vec2 mouse = (2.*iMouse.xy/iResolution.xy)-1.001;

float hash(float x){ return fract(sin(x)*43758.5453);}
float hash(vec2 x){ return hash(hash(x.x)+x.y); }
float hash(vec3 x){ return hash(hash(x.xy)*x.z); }

float smoothstepf(float x){ return smoothstep(0.,1.,fract(x)); }
vec2  smoothstepv2(vec2 x){ return vec2(smoothstepf(x.x),smoothstepf(x.y)); }

float hashsquare(vec2 x){
	x.y-=iGlobalTime/2.;
	x.x+=sin(iGlobalTime/4.)*2.;
	vec2 ix = floor(x);
	vec2 dx = smoothstepf(fract(x));
	
	float v1=hash(ix+vec2(0.,0.)),
		  v2=hash(ix+vec2(1.,0.)),
		  v3=hash(ix+vec2(0.,1.)),
		  v4=hash(ix+vec2(1.,1.));
	
	float h = mix(mix(v1,v2,dx.x),mix(v3,v4,dx.x),dx.y);	
	
	return h;
}

float spherize(vec2 uv){
	return dot(uv,uv)/300.;
}

float heightmap(vec2 x){
	
	const float scale=1.;
	
	x/=scale;
	float h=0.;
	
	mat3 p=mat3(vec3(4. ,0.03 ,0.),
				vec3(8. ,0.01 ,0.),
				vec3(68.,0.003,0.));
	

	h=hashsquare(x/4.+iGlobalTime*0.0)*2.;
	h=mix(h,hashsquare(x*p[0].x+iGlobalTime*p[0].z),p[0].y);
	h=mix(h,hashsquare(x*p[1].x+iGlobalTime*p[1].z),p[1].y);
	h=mix(h,hashsquare(x*p[2].x+iGlobalTime*p[2].z),p[2].y);
	//h=mix(h,hashsquare(x*2.),0.01);

	return h*scale-spherize(x*scale);	
}

vec3 tex(vec3 p, float mat){
	vec3 color;
	
	if(mat==1.){
		float b=1.-smoothstep(-0.,1.5,p.y+2.-spherize(p.xz));
		color=mix(vec3(0.7,0.5,0.2),vec3(0.5,0.5,0.3),b);
	}
	
	
	return clamp(color,0.,1.);
}

vec2 map(vec3 p){
	vec2 t1=vec2(p.y+heightmap(p.xz),1.);
	return t1;
}

vec3 normal(vec3 p){
	vec3 e=vec3(0.005,0.,0.);
	vec3 n=vec3(
		map(p+e.xyy).x-map(p-e.xyy).x,
		map(p+e.yxy).x-map(p-e.yxy).x,
		map(p+e.yyx).x-map(p-e.yyx).x);
	return normalize(n);
}

vec2 intersect(vec3 ro, vec3 rd){
	const int iter=200;
	const float mint=0.01;
	float t=0.;
	for(int i=0 ; i<iter ; i++){
		vec2 h = map(ro+t*rd);
		if(h.x<0.001) return vec2(t,h.y);
		t+=max(mint*t,h.x);
	}
	return vec2(0.);
}

vec3 shade(vec2 uv){
	
	vec3 ro=vec3(0.,0.,0.),
		 rd;//=normalize(vec3(uv,-1.1));
	
	float h=map(ro).x;
	vec3 nor=normal(ro-vec3(0.,1.,0.)*h);
	
	vec3 lookat=vec3(0.,0.,-1.5)-nor/2.;
	vec3 cdir=normalize(lookat-ro);
	vec3 r  = normalize(cross(cdir,vec3(0.,1.,0.)));
	vec3 u  = normalize(cross(r,cdir));
	
	
	rd = normalize(uv.x*r+uv.y*u+cdir);
	
	//rd=normalize(vec3(uv.x,-1.2,uv.y+0.1));
	
	ro.y-=map(ro)-0.5;
	
	vec2 t = intersect(ro,rd);
	vec3 color=vec3(0.);
	
	if(t.y>0.){
		vec3 p = ro+t.x*rd;
		vec3 n = normal(p);
		vec2 rot = vec2(cos(iGlobalTime/3.),sin(iGlobalTime/3.)*3.);
		vec3 l = normalize(vec3(rot.x,rot.y,rot.x));
		
		float amb=1.*n.y-p.y/100.;
		float dif=max(0.,dot(n,l));
		float spc=max(0.,dot((dot(rd,n)*n*-2.+rd),l));
		spc=smoothstep(0.85,1.,spc);
		
		color = vec3(0.3,0.15,0.7)*amb;
		color +=vec3(1.,1.,0.9)*dif;
		color +=spc;
		
		color *= tex(p,t.y);
	}
	
	return vec3(color);	
}

void main(void){
	vec2 uv = (2.*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
	gl_FragColor = vec4(shade(uv),1.0);
}
