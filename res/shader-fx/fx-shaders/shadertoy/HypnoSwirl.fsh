// https://www.shadertoy.com/view/4ds3WH
#define halfPhase 3.5
#define speed_modifier 1.5

void main(void) {

	float activeTime = iGlobalTime * speed_modifier;
	vec3 col; 
	float timeMorph = 0.0;
	
	#ifdef IS_IOS	
		vec2 p = -1.0 + 2.0 * gl_FragCoord.xy;	//iOS Dynamic AR App support
	#else
		vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
	#endif
	
	p *= 7.0;
	
	float a = atan(p.y,p.x);
	float r = sqrt(dot(p,p));
	
	if(mod(activeTime, 2.0 * halfPhase) < halfPhase)
		timeMorph = mod(activeTime, halfPhase);
	else
		timeMorph = (halfPhase - mod(activeTime, halfPhase));	
		
	timeMorph = 2.0*timeMorph + 1.0;
	
	float w = 0.25 + 3.0*(sin(activeTime + 1.0*r)+ 3.0*cos(activeTime + 5.0*a)/timeMorph);
	float x = 0.8 + 3.0*(sin(activeTime + 1.0*r)+ 3.0*cos(activeTime + 5.0*a)/timeMorph);
	
	col = vec3(0.1,0.2,0.82)*1.1;

	gl_FragColor = vec4(col*w*x,1.0);
}
