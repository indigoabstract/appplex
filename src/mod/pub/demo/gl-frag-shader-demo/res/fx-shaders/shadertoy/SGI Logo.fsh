// https://www.shadertoy.com/view/ldXGz2

// iChannel0: c1

#define PI 3.1415926

vec3 rY(float a,vec3 v)
{
	return vec3(cos(a)*v.x+sin(a)*v.z,v.y,cos(a)*v.z-sin(a)*v.x);
}

float cyl(vec3 p,vec3 dir,float l)
{
	float d=dot(p,dir);
	return max(-d,max(d-l,distance(p,dir*d)-0.13));
}

float sph(vec3 p)
{
	return length(p)-0.13;
}

float side(vec3 p)
{
	float l0=1.0,l1=0.7;
	vec2 l=vec2(-l0,0.0);
	return min(min(min(min(min(cyl(p,vec3(-1.0,0.0,0.0),l0),cyl(p-l.xyy,vec3(0.0,-1.0,0.0),l0)),
	     cyl(p-l.xxy,vec3(1.0,0.0,0.0),l1)),sph(p)),sph(p-l.xyy)),sph(p-l.xxy));
}

float s(vec3 p)
{
	float d=1e3;
	for(int i=0;i<6;++i)
	{
		d=min(d,side(p));
		p = (p-vec3(-0.3,-1.0,0.0)).zxy;
		p.z=-p.z;
	}
	return d;
}

vec3 cam(vec3 v)
{
	float t=mod(iGlobalTime,10.0),w=smoothstep(1.0,2.5,t)-smoothstep(5.0,5.5,t);
	return rY(-PI*0.25+sin(iGlobalTime*5.0)*0.1*w,rY(PI*0.19+cos(iGlobalTime*4.0)*0.1*w,v.yxz).yxz);
}

vec3 sceneNorm(vec3 rp)
{
	vec3 e=vec3(1e-3,0.0,0.0);
	float d0=s(rp);
	return normalize(vec3(s(rp+e)-d0,s(rp+e.yxy)-d0,s(rp+e.yyx)-d0));
}

void main(void)
{
	vec3 uv=gl_FragCoord.xyy/iResolution.xyy,ro=vec3(0.0,0.3,7.5),rd=normalize(vec3((uv.x-0.5)*2.0*iResolution.x/iResolution.y,uv.y*2.0-1.0,-5.0)),rp=vec3(0.0);

	ro=cam(ro)-vec3(1.3/2.0);
	rd=cam(rd);
	
	float t=0.0,d;
	for(int i=0;i<40;++i)
	{
		rp=ro+rd*t;
		d = s(rp);
		if(d < 1e-2)
			break;
		t += d*1.2;
	}
	
	gl_FragColor=sqrt(0.7*mix(vec4(0.0),textureCube(iChannel0,reflect(rd,sceneNorm(rp))),step(t, 1e1)));
}
