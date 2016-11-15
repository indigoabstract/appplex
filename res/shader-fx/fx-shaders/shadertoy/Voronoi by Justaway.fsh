// https://www.shadertoy.com/view/lsjGDd
float s;
void srand(vec2 p){
	s=sin(dot(p,vec2(423.62431,321.54323)));
}
float rand(){
	s=fract(s*32322.65432+0.12333);
	return abs(fract(s));
}
float grad(float t){
	return 6.0*pow(t,5.0)-15.0*pow(t,4.0)+10.0*pow(t,3.0);
}
mat2 rot2d(float a){
	float c=cos(a);
	float s=sin(a);
	return mat2(
		c,-s,
		s, c);
}
vec4 voronoi2d(vec2 p,float t){
	float v=8.0;
	vec4 c;
	vec2 f=floor(p)+vec2(0.5);
	for(float i=-3.0;i<3.0;i++)
	for(float j=-3.0;j<3.0;j++){
		srand(f+vec2(i,j));
		vec2 o;
		o.x=rand();
		o.y=rand();
		o*=rot2d(t*(rand()-0.5));
		float d=distance(p,f+vec2(i,j)+o);
		if(d<v){
			v=d;
			c.r=rand();
			c.g=rand();
			c.b=rand();
		}
	}
	return vec4(c*(1.0-v));
}
vec4 smoothvoronoi2d(vec2 p,float t){
	float v=0.0;
	vec2 f=floor(p)+vec2(0.5);
	for(float i=-3.0;i<3.0;i++)
	for(float j=-3.0;j<3.0;j++){
		srand(f+vec2(i,j));
		vec2 o;
		o.x=rand();
		o.y=rand();
		o*=rot2d(t*(rand()-0.5));
		float r=distance(p,f+vec2(i,j)+o);
		v+=exp(-8.0*r);
	}
	return vec4(-(1.0/8.0)*log(v));
}
void main(void){
	float t=iGlobalTime;
	float r=iResolution.x/iResolution.y;
	vec2 s=vec2(
		(gl_FragCoord.x-iResolution.x/2.0)/iResolution.x*r,
		(gl_FragCoord.y-iResolution.y/2.0)/iResolution.y);
	gl_FragColor=voronoi2d(s*5.0,t);
}
