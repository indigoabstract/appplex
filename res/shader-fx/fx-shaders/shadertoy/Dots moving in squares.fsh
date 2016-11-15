// https://www.shadertoy.com/view/ls23W3
// An attempt at recreating this gif: http://i.imgur.com/sX9SHHm.gif
float x(float t) { // From http://mathforum.org/kb/message.jspa?messageID=407257
	t = mod(t, 4.0);
	return abs(t) - abs(t-1.0) - abs(t-2.0) + abs(t-3.0) - 1.0;	
}

void main(void)
{
	vec2 uv = -0.5 + gl_FragCoord.xy / iResolution.xy;
	uv.x *= iResolution.x / iResolution.y;
	vec2 p = abs(mod(uv*30.0, 1.0));
	vec2 cell = floor(uv*30.0);
	float t = iGlobalTime*2.0+atan(cell.y+0.01,cell.x)/1.57*4.0+length(cell)/3.0;
	vec2 s = vec2(x(t), x(t-1.0))*0.35+0.5; 
	float d = max(abs(p.x-s.x), abs(p.y-s.y));
	gl_FragColor = vec4(smoothstep(0.15, 0.1, d));
}
