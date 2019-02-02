// https://www.shadertoy.com/view/4ssXzX
//fast vec2 -> vec4 hash function for webgl
//github.com/victor-shepardson

//hash based on Blum, Blum & Shub 1986
//and Sharpe http://briansharpe.wordpress.com/2011/10/01/gpu-texture-free-noise/
float pi = 3.14159265359;
float bbsm = 1739.;
vec2 bbsopt(in vec2 a){
	return fract(a*a*(1./bbsm))*bbsm;
}
vec2 mod1024(in vec2 a){
	return fract(a*(1./1024.))*1024.;	
}
vec4 hash(in vec2 pos){
	vec2 a0 = mod1024(pos*pi);
	vec2 a1 = bbsopt(a0);
	vec2 a2 = a1.yx + bbsopt(a1);
	vec2 a3 = a2.yx + bbsopt(a2);
	return fract((a2.xyxy + a3.xyyx)*(1./bbsm));
}

void main(void)
{
	const float scale = 1.; //scale up or down
	const float speed = 60.; //animation speed
	const int num = 1; //average multiple instances
	
	vec2 p = floor((gl_FragCoord.xy-speed*iGlobalTime)*scale);
	
	vec4 c = vec4(0.);
	for(int i=0; i<num; i++)
		c+= hash(p+float(i*1024));
	c*=1./float(num);
	
	gl_FragColor = vec4(c.x,c.y,c.z,c.w);
		
}
