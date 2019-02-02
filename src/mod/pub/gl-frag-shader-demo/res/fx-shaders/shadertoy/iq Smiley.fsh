// https://www.shadertoy.com/view/4dlXRf
//float segm( float a, float b, float c, float x );
float segm( float a, float b, float c, float x )
{
    return smoothstep(a-c,a,x) - smoothstep(b,b+c,x);
}

void main(void)
{
	
	vec2 reso = iResolution.xy;
	
	reso.x -= 185.0;
	
	vec3 col = vec3(1.0);
	vec2 uv = gl_FragCoord.xy / reso;
	gl_FragColor = vec4(uv,0.5+0.5*sin(iGlobalTime),1.0);

	uv.x -= .3;
	vec2 position =  (uv - 0.5) + 0.2*vec2(cos(iGlobalTime), sin(iGlobalTime)) ;
	
	vec2 z = position;
	
	float r = length(z);
	float a = atan(z.x, z.y);
	float s = 0.5 + 0.5*sin(a*13.0 + iGlobalTime);
	float d = 0.4 + 0.1*pow(s,3.5);
	float f = (r<d)?1.0:0.0;
	
	float b = 0.5 + 0.5*sin(4.0+(iGlobalTime*2.3));
	vec2 e = vec2(abs(z.x)-0.15, (z.y-0.1)*(1.0+10.0*b));
	float g = 1.0 - segm (0.05, 0.1, 0.01, length(e)*step(0.0,e.y));
	
	
	float k = 0.5 + 0.5*sin(20.0*iGlobalTime/1.5);
	vec2 m = vec2( z.x, (z.y+0.1)*2.0*(1.0+10.0*k));
	g *= 1.0-segm(0.06, 0.1, 0.01, length(m));
	
	
	col = mix( vec3(1.0), vec3(0.9, 0.8, 0.0)*g, f);
	
	gl_FragColor = vec4(col,1.0);
	
	
	//col = mix( vec3(1.0), vec3(0.9, 0.8, 0.0), f);
	
	
	
	
            	//if(length(z) > 0.3){
            		//gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
            	//}
            	
	
	
}
