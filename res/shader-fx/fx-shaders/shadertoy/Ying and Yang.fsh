// https://www.shadertoy.com/view/ldlSDn
void main(void)
{
	float PI = 3.14159265359;
	
	float as = iResolution.x / iResolution.y;
	
	vec2 coords = gl_FragCoord.xy / iResolution.xy;
	vec2 gcoords = vec2(as, 1.0)*(coords - vec2(0.5, 0.5));
	
	float r = 0.35*((1.0+sin(iGlobalTime))/2.0)+0.055;
	float thickness = 0.08;
	
	float fi = mod((atan(gcoords.y, gcoords.x)-((iGlobalTime*4.0)+PI)), PI) / (2.0*PI); // 0..1
			
	if (abs(distance(gcoords, vec2(0.0, 0.0)) - r) < thickness*fi) {
	  gl_FragColor = vec4(fi, fi, fi, 1.0);	
	} else {
	  gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);	
	}
	
}
