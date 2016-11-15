// https://www.shadertoy.com/view/4ss3R4
#ifdef GL_ES
precision highp float;
#endif

void main(void)
{
    float scale = floor(abs(mod(iGlobalTime*0.25, 6.0)))*0.25 + 0.25;
	
	scale += sin(iGlobalTime*0.0001);
	
    vec2 pos = gl_FragCoord.xy - 0.5 * iResolution.xy;
	
	float dist = pos.x*pos.x+pos.y*pos.y;
	
	float red   = sin(dist*0.1*scale+iGlobalTime)*0.5 + 0.5;
	float green = sin(dist*0.2*scale+iGlobalTime)*0.5 + 0.5;
	
	gl_FragColor = vec4(red, green, 0.4, 1.0);
}
