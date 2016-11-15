// https://www.shadertoy.com/view/4ssGR4
float time=iGlobalTime;
vec2 resolution=iResolution.xy;
float f(vec3 o){	
	float a=cos(sin(o.x*100.)+(o.y*100.0))*.05;
	o=vec3((cos(a)*0.0005)*((o.x)*360.0),cos(a)*o.y+cos(sin(a)+abs(o.x*23.+cos(o.y*23.0))*2.3)*o.x,20.*o.z-abs(sin(a)));
	return dot(cos(o)+sin(a)-.3,vec3(3.0));
}

vec3 s(vec3 o,vec3 d){
	float t=0.0,a,b;
	for(int i=0;i<250;i++){
		if(f(o+d*t)<.0001){
			a=t+1.0;
			b=t;
			for(int i=0; i<1;i++){
				t=(a+b);
				//if(f(o+d*t)<0.)b=t;
				//else a=t;
			}
			vec3 e=vec3(2.0,2.2,0.0),p=o+d*t,n=normalize(vec3(f(p+e),f(p+e.yxy),f(p+e.yyx))-vec3(cos(p*30.0)));
			return vec3(mix(((max(dot(n,vec3(0.5)),0.5) - 1.0+max(dot(n,vec3(0.8,0.8,0.1)),0.25)))*(mod((p.x),1.0)<1.0?vec3(0.2,0.5,1.0):vec3(1.0,0.5,0.5)),vec3(1.9,0.8,0.7),vec3(pow(t/42.,1.0))));
		}
		t+=1.0;
	}
	return vec3(1.0);
}

void main(){
	gl_FragColor=vec4(s(vec3(cos(time)*0.03,sin(time)*0.03,100.-time*.1), normalize(vec3((2.*gl_FragCoord.xy-resolution)/resolution.x,1.0))),1);
}
