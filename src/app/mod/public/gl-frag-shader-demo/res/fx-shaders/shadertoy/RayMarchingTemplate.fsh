// https://www.shadertoy.com/view/ldlSWn
const float PI=3.14159;
float time = iGlobalTime/1.;

vec2 mouse = (2.*iMouse.xy-iResolution.xy)/iResolution.y;

float hash(float x){ return fract(sin(x)*43758.5453);}
float hash(vec2 x){ return hash(hash(x.x)+x.y); }
float hash(vec3 x){ return hash(hash(x.xy)*x.z); }

vec3 tex(vec3 p, float mat){
	vec3 color;
	
	if(mat==0.){
		float d=5.;
		color=vec3(smoothstep(-0.01,0.01,sin(p.x*d)*sin(p.y*d)*sin(p.z*d)));
	}
	
	if(mat==1.){
		color=vec3(1.,0.6,0.3);
	}
	
	return clamp(color,0.,1.);
}

vec2 map(vec3 p){
	vec2 t0=vec2(5.-max(abs(p.x),max(abs(p.y),abs(p.z))),0.);
	vec2 t1=vec2(length(p)-1.,1.);
	if(t1.x<t0.x)t0=t1;
	return t0;
}

vec3 normal(vec3 p){
	vec3 e=vec3(0.001,0.,0.);
	vec3 n=vec3(
		map(p+e.xyy).x-map(p-e.xyy).x,
		map(p+e.yxy).x-map(p-e.yxy).x,
		map(p+e.yyx).x-map(p-e.yyx).x);
	return normalize(n);
}

vec2 intersect(vec3 ro, vec3 rd){
	const int iter=200;
	const float mint=0.0001;
	float t=0.;
	for(int i=0 ; i<iter ; i++){
		vec2 h = map(ro+t*rd);
		if(h.x<0.001) return vec2(t,h.y);
		t+=max(mint*t,h.x);
	}
	return vec2(0.);
}

vec3 shade(vec2 uv){
	
	vec3 ro=vec3(sin(mouse.x*PI/2.)*2.,sin(mouse.y*PI/2.)*2.,2.);
	ro.z=cos(length(ro.xy)*PI/2.)*0.1+1.9;
	
	vec3 lat=vec3(0.,0.,0.);
	vec3 dir = normalize(lat-ro);
	vec3 axr  = normalize(cross(dir,vec3(0.,1.,0.)));
	vec3 axu  = normalize(cross(axr,dir));
	vec3 rd = normalize(uv.x*axr+uv.y*axu+dir);
	
	vec2 t = intersect(ro,rd);
	vec3 color=vec3(1.,0.,1.);
	
	if(t.y>=0.){
		vec3 p = ro+t.x*rd;
		vec3 n = normal(p);
		vec3 l = normalize(vec3(1.,0.8,06));
		
		float amb=0.5+0.5*n.y;
		float dif=max(0.,dot(n,l));
		
		color  = amb*vec3(0.2);
		color += dif;
		color *= tex(p,t.y);
	}
	
	return vec3(color);	
}

void main(void){
	vec2 uv = (2.*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
	gl_FragColor = vec4(shade(uv),1.0);
}
