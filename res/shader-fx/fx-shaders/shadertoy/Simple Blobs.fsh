// https://www.shadertoy.com/view/ldfSR7
#define CUTOFF 0.23

void main(void)
{
	vec2 uv = gl_FragCoord.xy/iResolution.xy;
	uv.x *= 1000.0;
	uv.y *= 550.0;
	
	// blob setup	
	
	vec2 blob1pos = vec2(250.0, 180.0);
	float blob1size = 350.0;
	vec2 blob2pos = vec2(450.0, 200.0);
	float blob2size = 300.0;
	vec2 blob3pos = vec2(600.0, 260.0);
	float blob3size = 400.0;
	
	// movement
	
	blob1pos.x += sin(iGlobalTime*3.0) * 70.0;
	blob1pos.y += sin(iGlobalTime*1.2) * 90.0;
	blob2pos.x += sin(iGlobalTime*2.0) * 150.0;
	blob2pos.y += sin(iGlobalTime*3.0) * 20.0;
	blob3pos.y += sin(iGlobalTime*1.4) * 100.0;
	blob3pos.x += sin(iGlobalTime*1.7) * 100.0;
	
	// calc
	
	float blob1dist = clamp(distance(blob1pos, uv),0.0, blob1size);
	float blob1influence = abs(1.0 - blob1dist/blob1size);
	
	float blob2dist = clamp(distance(blob2pos, uv),0.0, blob2size);
	float blob2influence = abs(1.0 - blob2dist/blob2size);

	float blob3dist = clamp(distance(blob3pos, uv),0.0, blob3size);
	float blob3influence = abs(1.0 - blob3dist/blob3size);

	// finished
	
	float intensity = (pow(blob1influence,6.0) + pow(blob2influence,6.0) + pow(blob3influence,6.0));
	intensity = (clamp(intensity,CUTOFF, CUTOFF + 0.0125)-CUTOFF) * 80.0;
	
	gl_FragColor = vec4(intensity) - vec4(blob1influence,blob2influence,blob3influence,1.0);
}
