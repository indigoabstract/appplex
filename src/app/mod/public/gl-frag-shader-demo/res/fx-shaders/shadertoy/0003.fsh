// https://www.shadertoy.com/view/MsX3D4
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float aspect = iResolution.x / iResolution.y;
	vec2 uvn = uv * vec2(aspect, 1.0);

	float wobble2 = pow(uv.y, 2.0) * sin(iGlobalTime) * 0.02;
	float wobble = sin(abs(uv.x - 0.5 * wobble2) * 0.3 * sin(iGlobalTime) * 0.05);
	
	float y = 0.5 + sin(iGlobalTime) * 0.01;
	float strength = 500.0 + sin(sin(iGlobalTime)) * 100.0;
	float distort = sin(strength * sin(wobble)) * 0.2;
	
	y += distort;
	
	vec2 p0 = vec2(0.3 * aspect, y);
	vec2 p1 = vec2(0.7 * aspect, y);
	
	float d0 = length(p0 - uvn);
	float d1 = length(p1 - uvn);
	
	float d = min(d0, d1);
	
	d = 1.0 - smoothstep(0.2, 0.25, d);

	float sky = sin(iGlobalTime * 0.2) * 0.2;	
					
	gl_FragColor = vec4(d * 0.2, sky + d, 0.1, 1.0);
}
