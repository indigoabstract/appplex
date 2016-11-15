// https://www.shadertoy.com/view/4dlGWS

// iChannel0: t14
// iChannel1: c2
// iChannel2: t5

// Path Marching by eiffie (marchs the probable ray paths)
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

//quality settings (up these until your browser breaks) I use 128 for videos.
#define SAMPLES 8
#define RaySteps 48
#define maxBounces 8
//select a material, or create your own in the GetMaterial function
#define GLASS
//#define PLASTIC
//#define OPAQUE
//#define METAL
//#define MIRROR
//#define EMISSIVE
#define TAO 6.283
const float fov = 2.5,blurAmount = 0.01,focusDistance=2.0,maxDepth=4.0;//camera fiddling
const float FudgeFactor=1.0,HitDistance=0.001,lBloom=500.0;//you can guess these
const vec3 ior=vec3(1.0,1.52,1.0/1.52);//water=1.33,glass=1.52,diamond=2.42 (index of refr)
const vec2 ve=vec2(HitDistance*0.1,0.0);//delta for normal calc
int obj=0; //which object was hit
bool bColoring=false; //are we coloring?
vec4 orbitTrap; //for fractals, to grab a color

struct material {vec3 color;float refrRefl,difExp,spec,specExp;};

vec3 c1,c2;//vars for the blob thingies
float s1,s2,mld;
const float k=4.0;

float DE(in vec3 z0){
	float a=length(z0-c1)-s1;
	float b=length(z0-c2)-s2;
	float dF=z0.y+1.0;
	float dS=max(-z0.y-0.95,-log(exp(-k*a)+exp(-k*b))/k);
#ifdef EMISSIVE
	mld=min(mld,dS);//save minimum distance to emissive material
#endif
	dS=min(dS,dF);
	if(bColoring){//we only need to know the object and orbitTrap when coloring
		if(dS==dF)obj=1;
		else obj=0;
	}
	return dS;
}

material getMaterial( in vec3 p, inout vec3 nor )
{//return material properties corresponding to the hit obj
	if(obj==1){
		vec3 col=texture2D(iChannel2,p.xz*0.5).rgb;
		nor=normalize(nor+vec3(0.0,0.0,-col.b*0.25));
		return material(col,0.01,4.0+col.b*col.b*256.0,1.0,pow(2.,6.));
	}
#ifdef GLASS
	return material(vec3(0.5)+sin(-p.rgb)*cos(length(p)),-0.25,pow(2.,16.0),1.0,pow(2.,6.0));
#elif defined PLASTIC
	return material(vec3(0.5)+sin(-p.rgb)*cos(length(p)),-0.01,pow(2.,8.0),1.0,pow(2.,6.0));
#elif defined OPAQUE
	return material(vec3(0.5)+sin(-p.rgb)*cos(length(p)),0.01,4.0,1.0,pow(2.,6.0));
#elif defined METAL
	return material(vec3(0.5)+sin(-p.rgb)*cos(length(p)),0.01,pow(2.,6.0),1.0,pow(2.,6.0));
#elif defined MIRROR
	return material(vec3(0.5)+sin(-p.rgb)*cos(length(p)),0.8,pow(2.,16.0),1.0,pow(2.,6.0));
#elif defined EMISSIVE
	return material(vec3(0.0),0.01,1.0,1.0,pow(2.,16.0));
#endif
}

vec3 getBackground( in vec3 rd ){
	return vec3(0.8,1.1,1.2)*textureCube(iChannel1,rd).rgb+vec3(1.0,0.9,0.8)*pow(max(0.0,dot(rd,vec3(0.0,0.707,0.707))),80.0)*8.0;
}

//the code below can be left as is so if you don't understand it that makes two of us :)

//random seed and generator
vec2 randv2;
vec2 rand2(){// implementation derived from one found at: lumina.sourceforge.net/Tutorials/Noise.html and then f'd by me
	randv2+=vec2(1.0,1.0);//maybe the texture is faster but already had this code
	return vec2(fract(sin(dot(randv2.xy ,vec2(12.9898,78.233))) * 43758.5453),
		fract(cos(dot(randv2.xy ,vec2(4.898,7.23))) * 23421.631));
}
 
vec3 powDir(vec3 nor, vec3  dir, float power) 
{//creates a biased random sample without penetrating the surface (approx Schlick's)
	float ddn=max(0.01,abs(dot(dir,nor)));
	vec2 r=rand2()*vec2(TAO,1.0);
	vec3 nr=(ddn<0.99)?nor:((abs(nor.x)<0.5)?vec3(1.0,0.0,0.0):vec3(0.0,1.0,0.0));
	vec3 sdir=normalize(cross(dir,nr));
	r.y=pow(r.y,1.0/power);
	vec3 ro= normalize(sqrt(1.0-r.y*r.y)*(cos(r.x)*sdir + sin(r.x)*cross(dir,sdir)*ddn) + r.y*dir);
	return (dot(ro,nor)<0.0)?reflect(ro,nor):ro;
}

vec3 scene(vec3 ro, vec3 rd) 
{// find color of scene
	vec3 fcol=vec3(1.333),ael=vec3(0.0);//color frequency mask and accumulated emissive light
	vec2 rnd=rand2();//a couple random numbers
	float d,t=rnd.x*HitDistance*10.0,side=sign(DE(ro+t*rd));//dist to obj, total ray len, -inside or +outside of obj
	int iHitCount=0;mld=100.0;
	for(int i=0; i<RaySteps; i++ ){// march loop
		if(t>=maxDepth)continue;
		t+=d=DE(ro+t*rd)*side*FudgeFactor;//march
		if(abs(d)<HitDistance*t){//hit
			orbitTrap=vec4(1000.0);
			bColoring=true;//turn on material mapping
			t+=d=DE(ro+t*rd)*side;//move close to the hit point without fudging
			bColoring=false;
#ifdef EMISSIVE
			ael+=fcol/vec3(lBloom*mld*mld);//accumulate emissive light
#endif
			ro+=rd*t;// advance ray position to hit point
        	vec3 nor = side*normalize(vec3(-DE(ro-ve.xyy)+DE(ro+ve.xyy),
				-DE(ro-ve.yxy)+DE(ro+ve.yxy),
				-DE(ro-ve.yyx)+DE(ro+ve.yyx)));// get the surface normal
			material m=getMaterial( ro, nor );//and material
			rnd=rand2();//a couple more random numbers
			vec3 refl=reflect(rd,nor),newRay=refl;//setting up for a new ray direction and defaulting to a reflection
			if(m.refrRefl<0.0){//if the material refracts use the fresnel eq. to determine refr vs refl
				vec3 refr=refract(rd,nor,(side>=0.0)?ior.z:ior.y);//calc the probabilty of reflecting instead
				vec2 ca=vec2(dot(nor,rd),dot(nor,refr)),n=(side>=0.0)?ior.xy:ior.yx,nn=vec2(n.x,-n.y);
				if(rnd.y>0.5*(pow(dot(nn,ca)/dot(n,ca),2.0)+pow(dot(nn,ca.yx)/dot(n,ca.yx),2.0))){newRay=refr;nor=-nor;}
			}
			rd=powDir(nor,newRay,m.difExp);//redirect the ray
			//the next line calcs the amount of energy left in the ray based on how it bounced (diffuse vs specular) 
			fcol*=mix(m.color,vec3(1.0),min(pow(max(0.0,dot(rd,refl)),m.specExp)*m.spec+abs(m.refrRefl),1.0));
			t=max(d*5.0,HitDistance)+HitDistance*rnd.x*5.0;//hopefully pushs thru the surface
			if(iHitCount++>maxBounces || dot(fcol,fcol)<0.01)t=maxDepth;//bail out
			side=sign(DE(ro+t*rd));//keep track of which side you are on
			mld=100.0;
		}
	}
#ifdef EMISSIVE
	ael+=fcol/vec3(lBloom*mld*mld);//accumulate emissive light, runs twice some times, oh well
#endif
	return ael+fcol*getBackground(rd);//light the scene
}	

void main(void)
{
	//setup the blobs
	float time=iGlobalTime*0.05;
	c1=vec3(cos(time)*vec2(sin(time),cos(time)),sin(time))*0.5;
	c2=vec3(cos(time*2.0),sin(time*2.0)*vec2(sin(time*3.0),cos(time*3.0)))*0.75;
	s1=cos(time*4.0)*0.25+0.5;
	s2=1.0-s1;
	randv2=fract(cos((gl_FragCoord.xy+gl_FragCoord.yx*vec2(1000.0,1000.0))+vec2(iGlobalTime))*10000.0);
	vec2 size=iResolution.xy;
	vec3 eye=vec3(0.0,0.0,-2.0);
	vec3 clr=vec3(0.0);
	for(int iRay=0;iRay<SAMPLES;iRay++){
		vec2 pxl=(-size+2.0*(gl_FragCoord.xy+rand2()))/size.y;
		vec3 er = normalize( vec3( pxl.xy, fov ) );
		vec3 go = blurAmount*focusDistance*vec3( -1.0 + 2.0*rand2(), 0.0 );
       	vec3 gd = normalize( er*focusDistance - go );gd.z=0.0;//trying to push focal point and eye around
		clr+=scene(eye+go,normalize(er+gd));
		//clr+=scene(eye+rotCam*go,normalize(rotCam*(er+gd)));//this example doesn't rotate the camera
	}
	clr/=vec3(SAMPLES);
	gl_FragColor = vec4(clr,1.0);
}
