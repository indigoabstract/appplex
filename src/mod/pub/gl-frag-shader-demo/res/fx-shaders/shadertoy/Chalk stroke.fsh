// https://www.shadertoy.com/view/lslGR4

// iChannel0: t2
// iChannel1: t12

float distanceToSegment(vec2 a, vec2 b, vec2 pt)
{
	vec2 ab = b - a;
	vec2 ac = pt - a;
	vec2 bc = pt - b;
	
	float e = dot(ac, ab);
	if (e <= 0.0) 
		return length(ac);
	
	float f = dot(ab, ab);
	if (e >= f)
		return length(bc);
	
	return sqrt(dot(ac, ac) - e * e / f);
}

const vec2 A = vec2(0.0, 0.0);
const vec2 B = vec2(0.5, 0.5);
const float BRUSH_SIZE = 0.02;

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float dist = distanceToSegment(iMouse.xy/iResolution.xy, iMouse.zw/iResolution.xy, uv);
	float height = texture2D(iChannel0, uv).r;
	
	if (dist <= BRUSH_SIZE)
	{
		vec2 uvNoise = uv;
		//uvNoise.x += mod(iGlobalTime, 1.0);
		float noise = texture2D(iChannel1, uvNoise).r;
		float alpha = min(pow(smoothstep(0.0, 0.1 + 0.9 * noise, 1.0 - dist / BRUSH_SIZE), 1.4), 1.0);
		float col = height * alpha + height * 0.2;
		
		gl_FragColor = vec4(col);
	}
	else
	{
		gl_FragColor = vec4(height * 0.2);
	}
}
