// https://www.shadertoy.com/view/4df3Dr

// iChannel0: c0
// iChannel1: c2

void rotate(inout vec2 v, float a){v=cos(a)*v+sin(a)*vec2(v.y,-v.x);}
void kaleido(inout vec2 v,float p,inout float flr){//fold
	flr=floor(.5+atan(v.x,-v.y)*p*0.159155);
	rotate(v,flr*6.283185/p);
}
float time=iGlobalTime*0.1;
float Limb(vec3 p, vec3 p0, vec3 p2, vec3 rt, float d, float r){
	vec3 p1=(p2-p0)*0.5;//a simple joint "solver"
	p1+=p0+normalize(cross(p1,rt))*(d-dot(p1,p1));
	vec3 v=p1-p0;v*=clamp(dot(p-p0,v)/dot(v,v),0.0,1.0);
	vec3 v2=p1-p2;v2*=clamp(dot(p-p2,v2)/dot(v2,v2),0.0,1.0);
	return min(distance(p-p0,v),distance(p-p2,v2))-r;
}
float minDist=100.0;
vec3 rt=vec3(-1.0,0.0,0.0);
float DE(in vec3 z0){//tiled spiders
	vec3 z=z0;
	vec2 flr2=floor(z.xz*0.125)*10.0;
	z.xz=abs(vec2(4.0)-mod(z.xz,8.0))-vec2(2.0)+sin(flr2.yx)*0.75;
	float flr=0.0;
	kaleido(z.xz,9.0,flr);
	float sn=sin(time*50.0+(flr+flr2.x+flr2.y)*2.0),sn2=-0.1+sin(time*50.0+flr2.x+flr2.y)*0.1;
	vec3 p2=vec3(sn*0.1,-0.7+sn*0.3,-1.0+sn*0.25);
	float d=min(length(z-vec3(0.0,0.15+sn2,0.0))-0.2,Limb(z,vec3(0.0,sn2,0.0),p2,rt,1.1,0.025));
	minDist=min(minDist,d);
	return d;
}

vec3 lightDir=vec3(0.5,0.9,0.1);
float Shadow(vec3 ro){
	vec3 z=ro+0.5*lightDir;
	vec2 flr2=floor(z.xz*0.125)*10.0;
	z.xz=abs(vec2(4.0)-mod(z.xz,8.0))-vec2(2.0)+sin(flr2.yx)*0.75;
	float sn2=-0.1+sin(time*50.0+flr2.x+flr2.y)*0.1;
	return clamp(length(z-vec3(0.0,0.15+sn2,0.0))-0.1,0.0,1.0);
}

float rand(vec2 co){// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return fract(sin(dot(co,vec2(12.9898,78.233))) * 43758.5453);
}
void scene(vec3 ro, vec3 rd, inout vec3 color){
	vec3 bg=color;
	float rayLen=rand(gl_FragCoord.xy)*0.5,dist=10.0;
	ro+=rayLen*rd;
	for(int i=0;i<78;i++){
		rayLen+=dist=DE(ro)*0.45;
		ro+=dist*rd;
		if(rayLen>70.0 || ro.y<-1.0 || dist<0.01)break;
	}
	color*=min(minDist*3500.0/(1.0+rayLen*rayLen),1.0);
	if(rayLen<70.0){
		float shad=1.0;
		shad=Shadow(vec3(ro.x,-1.0-rand(gl_FragCoord.xy)*0.25,ro.z));
		color=mix(vec3(0.0),color*shad,smoothstep(0.0,0.065,dist));
		color=mix(color,bg,rayLen/75.0);
	}
}

void main(void)
{
	float tim=mod(iGlobalTime,40.0),c=0.0;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec3 ms=vec3((iMouse.xy/iResolution.xy-vec2(0.5))*vec2(5.0,2.0),0.0);
	if(iMouse.z<0.1)ms=vec3(0.0,0.2,0.0);
	vec3 rd=vec3(uv*vec2(1.0,iResolution.y/iResolution.x)-vec2(0.5)+ms.zy,0.5);
	rotate(rd.xz,ms.x);
	rd=normalize(rd);
	if(tim<20.0)c=textureCube(iChannel0,rd).r;
	else c=textureCube(iChannel1,rd.zyx).r;
	vec3 color=vec3(c*c);
	if(rd.y<0.0)scene(vec3(iGlobalTime,2.0,0.0),rd,color);//these spiders don't jump
	else if(tim<20.0){//candle flicker
		if(rd.z>0.9 && abs(rd.y)<0.1 && c>0.25){
			rd.y-=abs(sin((rd.x+iGlobalTime)*30.0)*0.005);
			c=max(c,textureCube(iChannel0,rd).r);
			color=vec3(c*c);
		}
	}
	gl_FragColor = vec4(color,1.0);
}
