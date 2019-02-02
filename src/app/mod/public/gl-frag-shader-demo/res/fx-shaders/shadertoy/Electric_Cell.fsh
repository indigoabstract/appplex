// https://www.shadertoy.com/view/ldsSRM

// iChannel0: m1

#define CUTOFF 0.23
float time=iGlobalTime*2.0;

float rand(float x)
{
	return fract(sin(x) * 43758.5453) ;
}

vec2 randpos(float x)
{
	return vec2(rand(x)*2.0-1.0, rand(x * 1.5)*2.0-1.0);
}


float cell(float size,vec2 basepos,vec2 uv,vec2 vel){

	float ss=size;
	vec2 pos = basepos;
	vec2 center = uv;
	float dis;
	float influence;
	
	pos.x=(basepos.x+sin(time*(0.3+rand(ss*1.6))))*
		  (rand(vel.x)*0.1+vel.x);
	pos.y=(basepos.y+cos(time*(0.35+rand(ss*1.2))))*
		  (rand(vel.y)*0.1+vel.y);	

	dis = clamp(distance(pos, uv),0.0, size);
	influence = 1.0 - (dis/ss);
    influence = pow(influence,4.0) ;
    return  influence;
}



void main(void)
{
	vec2 uv = 2.0*(gl_FragCoord.xy/iResolution.xy)-1.0;
	uv.x *= iResolution.x/iResolution.y;

	float color=0.0;
	float intensity=0.0;
	vec2 vel = vec2((texture2D( iChannel0, vec2(0.2,0.25) ).x),
					(texture2D( iChannel0, vec2(0.7,0.25) ).x)
			   );
	
		
	for(float k = 0.0; k<50.0 ; k++){
		
		float size=rand(k+1.0)*0.3+0.05;
		vec2 basepos =randpos(k+1.0);
		intensity += cell(size,basepos,uv,vel);
		
	}
	intensity = (clamp(intensity,CUTOFF, CUTOFF + 0.0125)-CUTOFF) * 80.0;
	color=intensity;
	color=1.0-color;
	

	float bg = sqrt((1.5 *fract(time*1.3))*length(uv));
	bg += fract(sin(dot(uv, vec2(344.4324, time))*5.3543)*2336.65)*0.5;
	
	color=color+bg;
	
	
	gl_FragColor = vec4(color,color,color ,1.0);
}
