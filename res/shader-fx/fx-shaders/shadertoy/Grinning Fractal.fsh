// https://www.shadertoy.com/view/Msf3WS
//based on code from fragmentarium
//http://syntopia.github.io/Fragmentarium/

vec3 fractal(vec2 c, vec2 c2) {	
	vec2 z = c;
	float ci = 0.0;
	float mean = 0.0;
	for(int i = 0;i < 64; i++) {
		vec2 a = vec2(z.x,abs(z.y));
		float b = atan(a.y, a.x);
		if(b > 0.0) b -= 6.283185307179586;
		z = vec2(log(length(a)),b) + c2;
		if (i>1) mean+=length(z);
	}
	mean/=float(62);
	ci =  1.0 - log2(.5*log2(mean/1.));
	return vec3( .5+.5*cos(6.*ci+0.0),.5+.5*cos(6.*ci + 0.4),.5+.5*cos(6.*ci +0.7) );
}


void main(void)
{
	vec2 uv = gl_FragCoord.xy - iResolution.xy*.5;
	uv /= iResolution.x;
	vec2 tuv = uv;
	float rot=sin(iGlobalTime*0.02)*2.7;		
	uv.x = tuv.x*cos(rot)-tuv.y*sin(rot);
	uv.y = tuv.x*sin(rot)+tuv.y*cos(rot);
	float juliax = sin(iGlobalTime) * 0.01 + 0.2;
	float juliay = cos(iGlobalTime * 0.23) * 0.02 + 5.7;
	gl_FragColor = vec4( fractal(uv, vec2(juliax, juliay)) ,1.0);
}
