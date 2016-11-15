// https://www.shadertoy.com/view/XdsXzM
void main(void)
{
	
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	uv -= 0.5;//center it
	uv *= 2.0;//screen domain is now -1 to 1
	uv.x *= iResolution.x/iResolution.y; //correct for aspect ratio by varying length of horizontal axis
	
	vec2 pA = vec2(1.0*sin(iGlobalTime),		1.0*cos(iGlobalTime));
	vec2 pB = vec2(1.0*sin(iGlobalTime*1.7),		0.8*cos(iGlobalTime*1.3));
	/* my original code
	float cA = length(uv-pA);
	cA = mod(cA*3.0,1.0);
	if(cA<0.5){cA=0.;}else{cA=1.;}
	
	float cB = length(uv-pB);
	cB = mod(cB*3.0,1.0);
	if(cB<0.5){cB=0.;}else{cB=1.;}
	
	float c = mod((cA + cB),2.); //GLSL has no XOR so I improvised this stupid thing
	*/
	
	// mmalex's version
	float cA = length(uv-pA);
	float cB = length(uv-pB);
	
	float c = sign(sin(cA*15.))*sign(sin(cB*15.));
	
	gl_FragColor = vec4(vec3(c),1.0);
}
