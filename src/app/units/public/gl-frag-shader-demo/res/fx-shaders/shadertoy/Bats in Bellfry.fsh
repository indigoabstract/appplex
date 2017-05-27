// https://www.shadertoy.com/view/lsf3Wn
//bats in bellfry by eiffie (low quality for speed)
//for a higher quality up the iters and steps and lower the "fudgefactor"
const float aoe=0.75,spec=0.25,specExp=64.0,maxD=4.0;
vec3 diffuse;
bool bColoring=false;
const float TAOd8=6.283*0.125,ooTd8=8.0/6.283;
float tim=iGlobalTime*0.1;
mat3 PYR(vec3 a){vec3 c=cos(a),s=sin(a);//pitch yaw roll
	return mat3(c.z*c.y+s.z*s.x*s.y,s.z*c.x,-c.z*s.y+s.z*s.x*c.y,-s.z*c.y+c.z*s.x*s.y,c.z*c.x,s.z*s.y+c.z*s.x*c.y,c.x*s.y,-s.x,c.x*c.y);
}
const vec4 scale=vec4(-2.0,-2.0,-2.0,2.0),p0=vec4(-0.46,-1.53,-1.79,0.0);
const vec3 rm=vec3(1.0,0.25,0.5625),dif=vec3(0.55,0.4,0.35);

float DE(vec3 z0)
{
	vec4 z = vec4(z0,1.0);//bellfry - amazingbox by tglad, with mods
	for (int n = 0; n < 9; n++) {
		z.zyx=clamp(z.xyz,-1.0,1.0)*2.0-z.xyz;
		if(z.y>z.x)z.xy=z.yx;
		z*=scale/clamp(dot(z.xyz*rm,z.xyz),0.5,1.0);
		z+=p0;
		if(bColoring && n==3)diffuse=dif+z.brg*0.02;
	}//bats - an attempt at unintelligible mayhem
	vec3 p=(z0-vec3(tim*2.0,-1.0-sin(z0.x+tim*3.7)*0.2,-1.0))*20.0;
	float a=p.x*0.07+sin(p.x)*0.2+tim*20.0;
	float s2=sin((p.x+p.y+p.z)*0.125);
	a=-a+floor(.5+(atan(p.z,-p.y)+a)*ooTd8)*TAOd8;
	p.x=abs(4.0-mod(p.x,8.0));
	p.zy=cos(a)*p.zy+sin(a)*vec2(p.y,-p.z);
	p+=vec3(-2.0+s2,5.0+s2,s2*0.5);
	a=-0.25+sin(tim*150.0+s2*2.0)*0.65;
	p.y=abs(p.y);
	p.xy=cos(a)*p.xy+sin(a)*vec2(p.y,-p.x);
	p*=2.0;
	float d=0.025*max(abs(p.x)-max(0.0,0.1-p.y*0.1),max(p.y-1.0,abs(p.z)-max(0.0,0.5-(p.y+sin((p.z+1.0)*3.1416)*0.5)*0.5)));
	float dS=min((length(z.xyz)-4.0)/z.w,length(z0.xy+vec2(-5.5,1.0))-0.025);
	if(d<dS){
		if(bColoring)diffuse=vec3(0.05,0.025,0.0);
		return d;
	}
	return dS;

}

void main() {
	vec2 ms=vec2(0.0);
	if(iMouse.z>0.5)ms=(iMouse.yx-0.5*iResolution.yx)/iResolution.yx;
	vec3 lightDir=normalize(vec3(0.8,-0.1,-0.1));
	vec3 ray = vec3(3.25,-1.0,-1.3), rayDir = normalize(
		vec3(0.5,(gl_FragCoord.xy/iResolution.xy-vec2(0.5))*vec2(1.0,float(iResolution.y)/iResolution.x))
		*PYR(vec3(0.2,vec2(0.15,0.0)+ms)));
	vec3 color=vec3(0.0);
	float fSteps=0.0;
	bool bHit=false;
	float rayLen=0.0,dist=maxD;
	for(int iSteps=0;iSteps<48;iSteps++){
		rayLen+=dist=DE(ray+rayLen*rayDir)*0.9;
		if(dist<0.001*rayLen*rayLen){bHit=true;break;}
		if(rayLen>maxD)break;
		fSteps+=1.0;
	}
	if(bHit){
		bColoring=true;
		rayLen+=DE(ray+rayLen*rayDir)-0.005;
		bColoring=false;
		ray+=rayLen*rayDir;
		vec2 ve=vec2(0.004,-0.004);
		vec3 normal=normalize(vec3(ve.xyy*DE(ray+ve.xyy)+ve.yyx*DE(ray+ve.yyx)+ve.yxy*DE(ray+ve.yxy)+ve.xxx*DE(ray+ve.xxx)));
		diffuse=(1.0-rayLen*rayLen*0.1)*max(-0.75*dot(normal,lightDir)+0.25,0.0)*diffuse+diffuse*0.125;//light from bottom
		if(length(ray.yz+vec2(1.0))<0.6)//moonlight
			diffuse+=rayLen*spec*pow(max(0.25+0.75*dot(lightDir,reflect(rayDir,normal)),0.0),specExp); 
		color=diffuse*(1.0 - (fSteps / 48.0)*aoe);
	}else if(rayLen>2.5)color=50.0*vec3(pow(max(dot(rayDir,lightDir),0.0),1024.0));
	gl_FragColor = vec4(clamp(color, 0.0, 1.0),1.0);
}
