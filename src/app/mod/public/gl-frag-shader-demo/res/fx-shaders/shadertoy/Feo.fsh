// https://www.shadertoy.com/view/XdsXW7
// Adding color to WAHa_06x36's shader https://www.shadertoy.com/view/XslSW7
#define SUPERSAMPLE

float rand(vec3 r) { return fract(sin(dot(r.xy,vec2(1.38984*sin(r.z),1.13233*cos(r.z))))*653758.5453); }

vec2 threshold(vec2 threshold,vec2 x,vec2 low,vec2 high) { return low+step(threshold,x)*(high-low); }

vec3 art(vec2 position)
{
	vec2 a=vec2(-1.0);
	vec2 b=vec2(1.0);
	float col=0.4;

	for(int i=0;i<7;i++)
	{
		vec2 m=(a+b)/2.0;
		vec2 d=b-a;

		if(rand(vec3(a,floor(iGlobalTime/2.0)+1.0))<0.7)
		{
			if(length(position-m)>length(d)*0.35) break;
			a+=d*0.15;
			b-=d*0.15;
			col=4.0*(1.0-col)*col;
		}
		else
		{
			a=threshold(m,position,a,m);
			b=threshold(m,position,m,b);
		}
	}
	return 0.5 + 0.5*cos( 80.0*col + vec3(0.0,1.0,3.0) );
}

void main(void)
{
	vec2 position=(2.0*gl_FragCoord.xy-iResolution.xy)/min(iResolution.x,iResolution.y);

	#ifdef SUPERSAMPLE
	float delta=1.0/min(iResolution.x,iResolution.y);
	vec3 col=(
		art(position+delta*2.*vec2(0.25,0.00))+
		art(position+delta*2.*vec2(0.75,0.25))+
		art(position+delta*2.*vec2(0.00,0.50))+
		art(position+delta*2.*vec2(0.50,0.75))
	)/4.0;
	#else
	float col=art(position);
	#endif

	gl_FragColor=vec4(vec3(col),1.0);
}
