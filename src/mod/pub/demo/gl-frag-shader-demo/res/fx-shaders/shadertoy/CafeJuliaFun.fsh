// https://www.shadertoy.com/view/MdsSW8
vec2 mouse = (2.*iMouse.xy-iResolution.xy)/iResolution.y;

float wdist;

float dist(vec2 uv){
	uv=abs(pow(abs(uv),vec2(sin(iGlobalTime)*0.5+0.7)));
	uv*=normalize(vec2(1.)+vec2(-1.,1.)*cos(iGlobalTime*1.7)*0.5+0.7);
	float d=max(uv.x,uv.y);
	return abs(d);	
}

vec2 warp(vec2 uv){
	uv=vec2(uv.x*uv.x-uv.y*uv.y , 2.*uv.x*uv.y);
	wdist=min(wdist,dist(uv));
	return uv;
}

vec3 shade(vec2 uv){
	
	vec2 wv=uv;
	
	wdist=dist(uv);
	
	for(int i=0 ; i<15 ; i++)
		wv = mouse+warp(wv);
	
	if(wdist>0.3) return vec3(0.2,0.2,0.3);
	
	wdist=abs(wdist-0.2+0.*sin(iGlobalTime));
	wdist=wdist-0.1;
	if(wdist<0.) wdist=sin(exp(-wdist*20.)*2.);
	else return vec3(0.25,0.25,0.4);
	
	
	float  d = smoothstep(0.,0.01,wdist);
	
	return vec3(0.7,0.4,0.5)*(0.3*(1.-d)+1.);	
}

void main(void){
	vec2 uv = (2.*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
	gl_FragColor = vec4(shade(uv),1.0);
}
