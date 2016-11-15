// https://www.shadertoy.com/view/4sfSzM
#define time iGlobalTime*.3

float makePoint(float i,float x,float y,float radius,float size,float t){
   	float tt=fract(t+fract(i/10.0)+floor(i/10.0)/10.0);
	float xx=x+cos(tt*3.141592*2.0)*radius*2.0;
   	float yy=y+sin(tt*3.141592*2.0)*radius;
	float aa=size/(xx*xx+yy*yy)*(2.0*abs(tt-0.5));
	aa=aa/20.0>1.0?4.0:fract(aa/50.0);
   	return aa;
}

float range(float value,float s1,float s2){
	return value*(s2-s1)+s1;
}

void main( void ) {

   	vec2 uv=(gl_FragCoord.xy/iResolution.xy)*2.0-1.0;
   	uv.x *= iResolution.x / iResolution.y;
	float a=0.0;
   
   	float x=uv.x-0.5;
   	float y=uv.y-0.5;

   	float t = fract(time);
	for(float i=0.0;i<100.0;i++){
  		a += makePoint(i,x+mod(i,10.0)/10.0,
				 y+(floor(i/10.0)/10.0),
				 0.2,0.1,t);
		
	}

   gl_FragColor = vec4(a/2.0,a/2.0,a/2.0,1.0);
}
