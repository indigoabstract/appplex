// https://www.shadertoy.com/view/ldlXDM

// iChannel0: t14

#define POINTS 150
#define epsilon 0.004

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec4 color;
	float minDist = 1.0;
	
	for (int i = 1; i <= POINTS; i++) {
		float K1 = float(i) / float(POINTS);
		float K2 = texture2D(iChannel0, vec2(0.0, float(i)/float(POINTS))).r;		
		
		K1 = sin((K1*iGlobalTime/4.0));
		
		vec2 xy = vec2(K1, K2);
		K2 = K2;
    	float dist = distance(uv, xy);	
		if (dist < minDist) {
		  minDist = dist;
		  color = vec4(K2, K2, 0.9 - K1, 1.0);
		}
		
		if (dist < epsilon) color = vec4(0.0);
	}
	
	gl_FragColor = color;
}
