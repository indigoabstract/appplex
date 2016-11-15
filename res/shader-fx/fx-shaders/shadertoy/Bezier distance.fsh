// https://www.shadertoy.com/view/XssXWn

// iChannel0: t10

const float PI=3.14159;
const float speed=0.0005;

const float sample=30.; // You should be able to increase both those value quite significantly
const int points=4;  // but it appears to break randomly on different architectures.

vec2 mouse = (2.*iMouse.xy-iResolution.xy)/iResolution.y;
float stime = sin(speed*iGlobalTime/PI)/2.+0.5;
vec2 uv;

// Order n bezier curve
struct Bez{
	vec2 p[points];
};

// Implicit smoothstep
float smoothstepf(float x){
	return smoothstep(0.,1.,fract(x))+floor(x);
}
vec2 smoothstepv2(vec2 x){
	return smoothstep(0.,1.,fract(x))+floor(x);
}
	
// Get noise value from vector
float hash(vec2 v){
	v=fract(v);
	v=(smoothstepv2(v*64.)+0.5)/64.;
	return texture2D(iChannel0,v).r;
}
	
// Get noise value from f in [0,1]
float hash(float f){
	f=fract(f);
	float r = iChannelResolution[0].y;
	float x = fract(f*r);
	float y = f-x;
	return hash(vec2(x,y));
}

// Distance/closest point from a segment
vec2 linep(vec2 a, vec2 b){
	a-=uv,
	b-=uv;
	vec2 dir = normalize(b-a);
	vec2 p = a+dir*clamp(dot(-a,dir),0.,length(b-a));
	return p;
		
}
vec2 linep(vec4 s){ return linep(s.xy,s.zw);}
float line(vec2 a, vec2 b){return length(linep(a,b));}
float line(vec4 s){ return line(s.xy,s.zw);}

// Dotted line. Terrible, terrible code, for your own sake don't look.
float dotline(vec2 a, vec2 b){
	a-=uv,
	b-=uv;
	vec2 dir = normalize(b-a);
	
	float x = dot(-a,dir);
	
	float dv=40.;
	float stretch = 0.5; // (0=circle dot , 1=line)
	
	float md=fract(iGlobalTime*-1.);
	
	x+=md/dv;
	
	float ix=floor(x*dv+0.5)/dv;
	float fx=fract(x*dv+0.5)-0.5;
	
	fx=clamp(fx,-stretch/2.,stretch/2.);
	x=clamp(ix+fx/dv-md/dv,0.,length(b-a));
	vec2 p = a+dir*x;
	
	return length(p);
}
	

// Bezier curve point with d in [0,1]
vec2 bezred(Bez b, float d){
	for(int x=points ; x>1 ; x--){
		for(int i=0 ; i<=points ; i++){
			if(i+1>=x) break;
			b.p[i]=mix(b.p[i],b.p[i+1],d);
		}
	}
	return b.p[0];
}

// closest Bezier control points
vec2 bezierp(Bez b){
	vec2 r;
	float d=-1.;
	for(int i=0 ; i<points ; i++){
		if(d==-1.||d>length(b.p[i]-uv)){
			d=length(b.p[i]-uv);
			r=b.p[i]-uv;
		}
	}
	return r;
}

// Closest distance from Bezier curve
vec2 bezier(Bez b){
	
	vec4 l=vec4(b.p[0],vec2(0.));
	float dist=-1.;
	float d=0.;
	vec2 p;
	
	for(float i=1. ; i<=sample ; i++){
		
		d=i/sample;
		
		l.zw=bezred(b,d);
		if(dist<0.){
			p=linep(l);
			dist=length(p);
		}else{
			vec2 t=linep(l);
			if(dist>length(t)){
				dist=length(t);
				p=t;
			}
		}
		l.xy=l.zw;
	}
	return p;
}
float bezierd(Bez b){ return length(bezier(b));}

// Randomish bezier generation
Bez beziergen(){
	Bez b;
	float vd=float(points);
	float ii = 0.;
	for(int i=0. ; i<points ; i++, ii += 1.){
		b.p[i]=vec2(hash(stime+ii/vd),hash(stime+(ii+0.5)/vd))*2.-1.;
	}
	return b;
}

// Basically the bezier distance again
float heightmap(){
	return bezierd(beziergen());
}

// Normalmap calculation. Horrible and extremely inefficient as well, because I set uv as global...
vec3 normalmap(){
	vec2 buv=uv;
	float eps=0.005;
	vec4 hs;
	
	uv=buv+eps*vec2(-1.,-1.);
	hs[0]=heightmap();
	uv=buv+eps*vec2( 1.,-1.);
	hs[1]=heightmap();
	uv=buv+eps*vec2(-1., 1.);
	hs[2]=heightmap();
	uv=buv+eps*vec2( 1., 1.);
	hs[3]=heightmap();
	uv=buv;
	
	float slopex=((hs[1]+hs[3])/2.-(hs[0]+hs[2])/2.);
	float slopey=((hs[2]+hs[3])/2.-(hs[0]+hs[1])/2.);

	return normalize(vec3(slopex,slopey,2.*eps));
}

// Color from pixel location
vec3 shade(){
	
	vec3 color;
	vec3 nm=normalmap();
	
	float d = heightmap();
	
	float lum=dot(nm,normalize(vec3(0.4)));
	
	
	lum = mix(lum,1.,0.8);
	
	color=nm/2.+0.5;
	
	color *= vec3(smoothstep(0.005,0.01,d));
	color += sin(d*200.)*0.01;
	
	vec2 bp=bezierp(beziergen());
	
	bp=abs(bp);
	vec2 bpx = vec2(bp.x,max(0.,bp.y-0.025));
	vec2 bpy = vec2(max(0.,bp.x-0.025),bp.y);
	
	//float bd = sqrt(bp.x)+sqrt(bp.y);
	
	float bd = min(length(bpx),length(bpy));
	
	color = abs(color-smoothstep(0.01,0.005,bd));
	
	vec2 buv=uv;
	uv=mouse;
	vec2 mp=bezier(beziergen())+uv;
	uv=buv;
	float md=dotline(mouse,mp);
	
	color *= smoothstep(0.003,0.006,md);
	
	return color;
}

void main(void)
{
	uv = (2.*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
	gl_FragColor = vec4(shade(),1.);
}
