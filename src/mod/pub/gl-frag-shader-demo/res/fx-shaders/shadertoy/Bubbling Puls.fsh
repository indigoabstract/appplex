// https://www.shadertoy.com/view/Xdl3zH
#define s(v) v.x+v.y+v.z
#define pi 3.14159265
#define R(p, a) p=cos(a)*p+sin(a)*vec2(p.y, -p.x)
#define hsv(h,s,v) mix(vec3(1.), clamp((abs(fract(h+vec3(1., 1., 3.)/3.)*6.-3.)-1.), 0., 1.), s)*v

const float BLOWUP=86.0; 
const float MAXSTEPSHIFT=10.0; 
const int MAXITERS=55;


float pn(vec3 p) { //noise @Las^Mercury
	vec3 i = floor(p);
	vec4 a = dot(i, vec3(1., 57., 21.)) + vec4(0., 57., 21., 78.);
	vec3 f = cos((p-i)*pi)*(-.5) + .5;
	a = mix(sin(cos(a)*a), sin(cos(1.+a)*(1.+a)), f.x);
	a.xy = mix(a.xz, a.yw, f.y);
	return mix(a.x, a.y, f.z);
}

float fpn(vec3 p) {
	return pn(p*.06125)*.5 + pn(p*.125)*.25 + pn(p*.25)*.125;
}

float displace(vec3 p) {
	return ((cos(4.*p.x)*sin(4.*p.y)*sin(4.*p.z))*cos(30.1))*sin(iGlobalTime);
}

int f(vec3 pos,float stepshift)
{
	float d = displace(pos);
	vec3 v2=abs(fract(pos + d) -vec3(0.5,0.5,0.5))/2.0;
	float noise = fpn(v2*130.+ iGlobalTime*05.) * 0.05;

	v2 = v2 + noise +d/2.;
	float r=0.0769*sin(iGlobalTime*30.0*-0.0708);
	float blowup=BLOWUP/pow(2.0,stepshift+8.0);

	if(s(v2)-0.1445+r<blowup) return 1;
	v2=vec3(0.25,0.25,0.25)-v2;
	if(s(v2)-0.1445-r<blowup) return 2;

	int hue;
	float width;
	if(abs(s(v2)-3.0*r-0.375)<0.03846+blowup)
	{
		width=0.1445;
		hue=4;
	}
	else
	{
		width=0.0676;
		hue=3;
	}

	if(s(abs(v2.zxy-v2.xyz))-width<blowup) return hue;

	return 0;
}


#define s(v) v.x+v.y+v.z
float time;
vec2 size;

	
void main(void)
{
	time    = iGlobalTime;
	vec2 uv = iResolution.xy;
	float x = 0.5*( 2.0 * gl_FragCoord.x - uv.x) / max( uv.x, uv.y);
	float y = 0.5*( 2.0 * gl_FragCoord.y - uv.y) / max( uv.x, uv.y);

	float sin_a = sin( time * 20.0 * 0.00564 );
	float cos_a = cos( time * 20.0 * 0.00564 );

	vec3 dir=vec3(x,-y,0.33594-x*x-y*y);
	dir=vec3(dir.y,dir.z*cos_a-dir.x*sin_a,dir.x*cos_a+dir.z*sin_a);
	dir=vec3(dir.y,dir.z*cos_a-dir.x*sin_a,dir.x*cos_a+dir.z*sin_a);
	dir=vec3(dir.y,dir.z*cos_a-dir.x*sin_a,dir.x*cos_a+dir.z*sin_a);

	vec3 pos=vec3(0.5,2.1875,0.875)+vec3(1.0,1.0,1.0)*0.0134*20.0*time;

	float stepshift=MAXSTEPSHIFT;

	if(fract(pow(x,y)*time*30.0*1000.0)>0.5) pos+=dir/pow(2.0,stepshift);
	else pos-=dir/pow(2.0,stepshift);

	int i=0;
	int c;

	for(int j=0;j<100;j++)
	{
		c=f(pos,stepshift);
		if(c>0)
		{
			stepshift+=1.0;
			pos-=dir/pow(2.0,stepshift);
		}
		else
		{
			if(stepshift>0.0) stepshift-=1.0;
			pos+=dir/pow(2.0,stepshift);
			i++;
		}

		if(stepshift>=MAXSTEPSHIFT) break;
		if(i>=MAXITERS) break;
	}

	vec3 col;
	if(c==0) col=vec3(0.0,0.0,0.0);
	else if(c==1) col=vec3(0.0,0.0,1.0);
	else if(c==2) col=vec3(0.0,1.0,0.0);
	else if(c==3) col=vec3(1.0,1.0,0.25);
	else if(c==4) col=vec3(0.5,0.5,0.5);

	float k=1.0-(float(i)-stepshift)/42.0;
	gl_FragColor=vec4(col*vec3(k*k,k*k*k,k*k*k),1.0);
}
