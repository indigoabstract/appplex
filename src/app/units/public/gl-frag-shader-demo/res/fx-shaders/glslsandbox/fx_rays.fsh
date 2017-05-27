// http://glsl.heroku.com/e#9934.3
#ifdef GL_ES
precision highp float;
#endif
//Ashok Gowtham M
//UnderWater Caustic lights
uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

//normalized sin
float sinn(float x)
{
	return sin(x)/2.+.5;
}

float CausticPatternFn(vec2 pos)
{
	return (sin(pos.x*40.+u_v1_time)
		+pow(sin(-pos.x*130.+u_v1_time),1.)
		+pow(sin(pos.x*30.+u_v1_time),2.)
		+pow(sin(pos.x*50.+u_v1_time),2.)
		+pow(sin(pos.x*80.+u_v1_time),2.)
		+pow(sin(pos.x*90.+u_v1_time),2.)
		+pow(sin(pos.x*12.+u_v1_time),2.)
		+pow(sin(pos.x*6.+u_v1_time),2.)
		+pow(sin(-pos.x*13.+u_v1_time),5.))/2.;
}

vec2 CausticDistortDomainFn(vec2 pos)
{
	pos.x*=(pos.y*.20+.5);
	pos.x*=1.+cos(u_v1_time/1.)/10.;
	return pos;
}

void main( void ) 
{
	//vec2 pos2 = gl_FragCoord.xy/u_v2_dim;
	vec2 pos = v_v2_tex_coord.xy;
	pos-=.5;
	vec2  CausticDistortedDomain = CausticDistortDomainFn(pos);
	float CausticShape = clamp(7.-length(CausticDistortedDomain.x*20.),0.,1.);
	float CausticPattern = CausticPatternFn(CausticDistortedDomain);
	float CausticOnFloor = CausticPatternFn(pos)+sin(pos.y*100.)*clamp(2.-length(pos*2.),0.,1.);
	float Caustic;
	Caustic += CausticShape*CausticPattern;
	Caustic *= (pos.y+.5)/4.;
	//Caustic += CausticOnFloor;
	float f = length(pos+vec2(-.5,.5))*length(pos+vec2(.5,.5))*(1.+Caustic)/1.;
	
	
	gl_FragColor = vec4(.1,.5,.6,1)*(f);
}
