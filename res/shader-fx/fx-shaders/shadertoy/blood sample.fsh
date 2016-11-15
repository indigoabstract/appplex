// https://www.shadertoy.com/view/XdfGR7
#define R(p,a) p=p*cos(a)+vec2(-p.y,p.x)*sin(a);

float sphere(vec3 p, float r){
	return dot(p,p)-r;
}

float box(vec3 p, vec3 r){
	vec3 di = abs(p)-r;
	return min( max(di.x,max(di.y,di.z)), length(max(di,0.0)) );
}

float scene(vec3 p){
	float h = sphere(mod(p,0.25)-0.125,0.01);
	float a = iGlobalTime+max(abs(cos(mod(iGlobalTime,acos(-1.0)))),0.0);
	float b1 = sphere(p-vec3(0.4*cos(iGlobalTime),0.0,-0.17),0.02);
	float b2 = sphere(p-vec3(0.0,0.17*cos(iGlobalTime+1.01),-0.17),0.02);
	R(p.xy,a*1.3);
	R(p.yz,a*0.7);
	float t = sphere(mod(p,0.25)-0.125,0.005);
	float c = max(box(p,vec3(0.25)),-h);
	float m = 1.0/(b1 + 0.021) + 1.0/(b2 + 0.021) + 1.0/(c + 0.026);
	float s = 1.0/m - 0.02;
	
	return max(max(s,-s-0.002),-t);
}

void main(void)
{
	vec2 pos = (gl_FragCoord.xy-0.5*iResolution.xy) / min(iResolution.x,iResolution.y);
	vec3 o = vec3(0.0,0.0,-min(iResolution.x,iResolution.y)/max(iResolution.x,iResolution.y));
	vec3 d = vec3(pos,0.0)-o;
	vec3 p = o;
	float l;
	float e = 0.001;
	vec3 c = vec3(0.0);
	float t = 0.0;
	for(int i = 0; i<253; i++){
		l=scene(p);
		if(abs(l)<e){
			vec3 lg = vec3(2.0,2.0,-13.3);
			c = vec3(t,0.0,0.05)*scene(p+0.1*lg);
			break;
		}
		t += 1.0/253.0;
		p += l*d;
	}
	gl_FragColor = vec4(c*4.0,1.0);
}
