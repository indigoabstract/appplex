// https://www.shadertoy.com/view/MssGR8

// iChannel0: t4

float gray(vec4 color)
{
	return (color.r + color.g + color.b) / 3.0;
}
	
void main(void)
{
	float pixelwide = 1.0 / iResolution.x;
    float pixelhigh = 1.0 / iResolution.y;

	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec4 c = texture2D(iChannel0, uv);
	float c_value = gray(c);
	
	vec4 l = texture2D(iChannel0, uv + vec2(-pixelwide, 0.0));
	vec4 u = texture2D(iChannel0, uv + vec2(0.0, pixelhigh));
	vec4 r = texture2D(iChannel0, uv + vec2( pixelwide, 0.0));
	vec4 b = texture2D(iChannel0, uv + vec2(0.0, -pixelhigh));
	
	float difference = 0.0;
	
	difference = max(difference, abs(c_value - gray(l)));
	difference = max(difference, abs(c_value - gray(u)));
	difference = max(difference, abs(c_value - gray(r)));
	difference = max(difference, abs(c_value - gray(b)));
	
	difference *= 20.0;
	
	gl_FragColor = vec4(difference, difference, difference, 1.0);
}
