// https://www.shadertoy.com/view/4dXSzH
#define time iGlobalTime*.5
#define resolution iResolution.xy

vec3 ldir;
float ot;

float julia(vec2 p) {
	p=p*2.+vec2(0.,-0.12);
	ot=1000.;
	float z = 0.0;
	int iter;
	for (int i=0; i<18; i++) {
		float l=dot(p,p);
		ot=min(ot,abs(l-.6));
		iter = i;
		if(l>4.0)
			break;
		float x = p.x*p.x-p.y*p.y;
		float y = 2.*p.x*p.y;
		p = vec2(x,y)+vec2(0.285,0.01);  
	}
	return dot(p,p)*.2;
}

float light(vec2 p) {
	vec2 d=vec2(0.,.003);
	float d1=julia(p-d.xy)-julia(p+d.xy);
	float d2=julia(p-d.yx)-julia(p+d.yx);	
  	vec3 n1=vec3(0.,d.y,d1);
  	vec3 n2=vec3(d.y,0.,d2);
  	vec3 n=normalize(cross(n1,n2));
  	float diff=max(0.,dot(ldir,n))*.6;
	vec3 r=reflect(vec3(0.,0.,1.),ldir);
	float spec=pow(max(0.,dot(r,n)),25.)*.4;
  	return (diff+spec+.15)*max(0.4,1.-julia(p));
}

void main( void )
{
	vec2 p = gl_FragCoord.xy/resolution.xy-.5;
	vec2 aspect=vec2(resolution.x/resolution.y,1.);
  	p*=aspect;
	vec3 lightpos=vec3(sin(time*3.)*.8,cos(time)*.9,-1.);
	lightpos.xy*=aspect*.5;
	ldir=normalize(vec3(p,-julia(p))+lightpos);
  	float l=light(p);
  	ot=max(1.-ot*.7,0.);
	vec3 col=l*vec3(ot*ot*1.45,ot*.6,ot*ot*.75);
	col+=pow(max(0.,.2-length(p+lightpos.xy))/.2,5.);
	col*=pow(max(0.,1.-length(p+lightpos.xy)*.3),2.5);
	gl_FragColor = vec4(col+.03, 1.0 );
}
