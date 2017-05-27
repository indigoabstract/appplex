// https://www.shadertoy.com/view/lsf3Dr

// iChannel0: t5

//eiffie - this is an example of fractal lighting but not a particulary good example
//the DE here is discontinuous so I tried to make the light falloff before hitting a cut
#define TAO 6.283
float pwrDvTao,taoDvPwr;
float Kaleido(inout vec2 v){float f=floor(.5+atan(v.x,-v.y)*pwrDvTao),a=f*taoDvPwr;v=cos(a)*v+sin(a)*vec2(v.y,-v.x);return f;}

float tim=iGlobalTime*0.2,minLiteDist=1000.0;
vec3 LightColor=vec3(1.0,0.7,0.5),noyzOffset=vec3(0.0,tim,0.0);

float rand(vec3 co){// implementation found at: lumina.sourceforge.net/Tutorials/Noise.html
	return fract(sin(dot(co*0.123,vec3(12.9898,78.233,112.166))) * 43758.5453);
}
float noyz(vec3 co){
	vec3 d=smoothstep(0.0,1.0,fract(co));
	co=floor(co);const vec2 v=vec2(1.0,0.0);
	return mix(mix(mix(rand(co),rand(co+v.xyy),d.x),
		mix(rand(co+v.yxy),rand(co+v.xxy),d.x),
		d.y),mix(mix(rand(co+v.yyx),rand(co+v.xyx),d.x),
		mix(rand(co+v.yxx),rand(co+v.xxx),d.x),d.y),d.z);
}

float DEL(in vec3 z0){//distance estimated light (works better if continuous;)
	vec4 z=vec4(z0,1.0);
	float t=tim,tN;
	for(int i=0;i<5;i++){
		float f=Kaleido(z.xz);
		tN=1.0+abs(sin(1.44+f))*0.2;
		if(t<tN)break;
		t-=tN;
		z*=2.0;
		z.z+=2.0;
	}
	t=t/tN;
	z.z+=t;
	z.y-=0.25+abs(sin(t*3.1416)*3.0)+(0.25-t*0.125)/z.w;
	return 0.5*(length(z.xyz)-0.25+t*0.125+noyz((noyzOffset+z0)*(20.0+tim*tim*2.0))*0.2)/z.w;
}

float DE(in vec3 z0)
{
	float dL=DEL(z0);
	minLiteDist=min(minLiteDist,dL);
	return min(z0.y,dL);
}

vec3 scene( vec3 ro, vec3 rd )
{
	vec3 color=vec3(0.0);
	float t=0.0,d=1.0;
	for(int i=0;i<48;i++){
		t+=d=DE(ro+rd*t);
		if(d<0.001 || t>7.0)break;
	}
	ro+=rd*t;
	if(t<7.0 && ro.y<0.001){
		vec2 pt=ro.xz*0.25;
		vec3 dif1=texture2D(iChannel0,pt).rgb;
		vec3 dif2=texture2D(iChannel0,pt+vec2(0.005,0.0)).rgb,dif3=texture2D(iChannel0,pt+vec2(0.0,0.005)).rgb;
		vec3 nor=normalize(vec3((dif2.r-dif1.r)*0.3,0.7,(dif3.r-dif1.r)*0.3));
		vec2 ve=vec2(DEL(ro),0.0);
		vec3 liteDir=normalize(vec3(DEL(ro-ve.xyy)-DEL(ro+ve.xyy),
			DEL(ro-ve.yxy)-DEL(ro+ve.yxy),DEL(ro-ve.yyx)-DEL(ro+ve.yyx)));
		float liteFalloff=1.0/max(ve.x*ve.x*(10.0+tim*tim*100.0),1.0);
		vec3 lDir2=vec3(-3.0,1.5,-2.0)-ro,refl=reflect(rd,nor);
		float d=length(lDir2),fo2=1.0/(d*d);
		lDir2/=d;
		dif1=(dif1*0.5+(dif2+dif3)*0.25)*(max(liteFalloff*dot(nor,liteDir),0.0)+fo2*dot(nor,lDir2));
		dif1+=liteFalloff*LightColor*pow(max(0.0,dot(liteDir,refl)),32.0)+fo2*LightColor*pow(max(0.0,dot(lDir2,refl)),32.0);
		color=dif1;
	}
	return max(color,LightColor/max(0.5,minLiteDist*minLiteDist*(1000.0+tim*tim*1000.0)));	
}	 

void main() {
	vec2 uv=(gl_FragCoord.xy/iResolution.xy-vec2(0.5))*vec2(1.0,iResolution.y/iResolution.x);
	vec3 ro=vec3(0.5,3.0,-3.0),tg=vec3(0.0,0.5,0.0);
	vec3 ww=normalize(tg-ro),uu=normalize(cross(ww,vec3(0.0,1.0,0.0))),vv=normalize(cross(uu,ww));
	vec3 rd=normalize(uv.x*uu+uv.y*vv+0.5*ww);
	float t=mod(tim,16.0);
	if(t>8.0){pwrDvTao=5.0/TAO;taoDvPwr=TAO/5.0;}
	else {pwrDvTao=3.0/TAO;taoDvPwr=TAO/3.0;}
	tim=mod(tim,8.0);
	vec3 color=scene(ro,rd);
	gl_FragColor = vec4(color,1.0);
}
