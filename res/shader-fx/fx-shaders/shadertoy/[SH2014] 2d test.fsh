// https://www.shadertoy.com/view/Xd23Dd
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec2 p = vec2(0.0, -1.0) + 2.0*uv;
	p.x *= iResolution.x/iResolution.y;
	p *= 10.0;
	vec2 wave1 = vec2(p.x, p.y + 3.0*sin(p.x*0.4 + iGlobalTime));
	vec2 wave2 = vec2(p.x, p.y + 3.0*sin(p.x*0.36+ 2.9 + iGlobalTime));
	float t = 1.0 - abs(wave1.y);
	t = max(t, 1.0 - abs(wave2.y));
	
	if (wave1.y * wave2.y < 0.0) t = mod(p.x + 3.141592654*iGlobalTime, 1.0);
	float c = t;
	c = smoothstep(0.7, 0.84, c);
	gl_FragColor = vec4(c,c,c,1.0);
}
