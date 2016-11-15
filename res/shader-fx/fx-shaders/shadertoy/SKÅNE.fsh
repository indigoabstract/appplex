// https://www.shadertoy.com/view/lssXW4

// iChannel0: t10

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv *= 2.0;
	uv -= 1.0;	
	
	
	vec4 bgc = texture2D(iChannel0, uv*10.0);
	float p = sin(uv.x*4.0+uv.y+iGlobalTime*2.0)*0.1;
	uv += p;
	
	
	bgc *= 1.0-distance(uv, vec2(0.0))*0.8;
	
	
	
	gl_FragColor = vec4(0.0);
	gl_FragColor = 1.0-abs(uv.y) > 0.8 ? vec4(1.0,1.0,0.0,1.0):vec4(1.0,0.0,0.0,1.0);
	gl_FragColor = 1.0-abs(uv.x+0.3) > 0.8 ? vec4(1.0,1.0,0.0,1.0):gl_FragColor;
	
	gl_FragColor -= p*4.0;
	
	gl_FragColor = 1.0-sqrt(pow(abs(uv.x),16.0)+pow(abs(uv.y),16.0))>0.9?gl_FragColor:bgc;
}
