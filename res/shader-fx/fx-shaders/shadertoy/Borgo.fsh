// https://www.shadertoy.com/view/ldf3zN
float time=iGlobalTime;
vec2 resolution=iResolution.xy;
float f(vec3 o){	
	float a=cos(o.x*8.)+sin(o.y*8.)+sin(o.z*1.)*12.5-(time*1.1);
	float b=length(sin(o.xy)+sin(o.yz)+sin(o.zx));
	o=vec3(cos(a)+o.x,1.-sin(a)*o.y,sin(a)*o.z)*.5;
	return mix(dot(cos(o)*cos(o),vec3(1.75))-2.0,b,.5);
}

vec3 s(vec3 o,vec3 d){
	float t=0.,a,b;
	for(int i=0;i<128;i++){
		if(f(o+d*t)<.1){
			a=t+1.0;
			b=t;
			for(int i=0; i<1;i++){
				t=a+b;
				if(f(o+d*t)<0.)
				b=t;
				else a=t;
			}
	
			return vec3(mix(vec3(0.55,0.1,0.18),vec3(0.1,0.15,0.2),vec3(pow(t/64.,1.0))));
	
		}
		t+=1.;
	}
	return vec3(0.1,0.25,0.23);
}

void main(){
	gl_FragColor=vec4(s(vec3(cos(1.6+cos(time*.1)),sin(.25+sin(time*.1)),-5.*sin(time*.01)*2.),normalize(vec3((gl_FragCoord.xy-resolution/2.)/resolution.x,1.0))),1.0);
}
