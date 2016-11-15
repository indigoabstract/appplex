// https://www.shadertoy.com/view/Ms2GWc
// Small change in https://www.shadertoy.com/view/ls23W3

// iChannel0: t4

float x(float t) { // From http://mathforum.org/kb/message.jspa?messageID=407257
	t = mod(t, 4.0);
	return abs(t) - abs(t-1.0) - abs(t-2.0) + abs(t-3.0) - 1.0;	
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	// uv.x *= iResolution.x / iResolution.y;
	vec2 p = abs(mod(uv*100.0, 1.0));
	vec2 cell = floor(uv*100.0);
	float t = iGlobalTime*7.0 + atan(cell.y+0.01,cell.x)/1.57*4.0+ length(texture2D(iChannel0,uv));
	t *= 1.5;
	vec2 s = vec2(x(t), x(t-1.0))*0.35+0.5; 
	float d = max(abs(p.x-s.x), abs(p.y-s.y))* 0.9725;
	gl_FragColor = vec4(smoothstep(0.25, 0.051, d))* texture2D(iChannel0,cell / 100.0).bbra * texture2D(iChannel0,cell / 100.0).rbra * 3.2 ;
}
