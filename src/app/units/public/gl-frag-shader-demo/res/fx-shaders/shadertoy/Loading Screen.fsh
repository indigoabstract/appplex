// https://www.shadertoy.com/view/ldXSRr

// iChannel0: t4
// iChannel1: t7

void main(void)
{
	
	float bpm = 175.0;
	
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv -= 0.5;
	uv.x *= iResolution.x / iResolution.y;
	
	float d = sqrt(distance(uv, vec2(0, 0)));
	float ang = atan(uv.y, uv.x) / 3.141592;
	
	float t = iGlobalTime;
	float bpmK = bpm / 120.0;
	float tk = bpmK * t;
	float f = fract(tk);
	float ff = fract(tk * 0.125);
	float ff2 = fract((t-0.1) * bpmK * 0.125);
	t += 3.0 * ceil(f - 0.25) + ceil(f - 0.75) + sin(ceil(t * bpmK)) * 100.0;
	float t1 = t + sin(t);
	float t2 = t * 0.8 + sin(t * 0.8 + 2.0);
	float t3 = t * 1.5 + 2.0 * sin(t * 0.4 + 2.0);
	
	float angSpeed = sin(sin(ceil(t * bpmK)) * 100.0);
	angSpeed = max(abs(angSpeed) * 3.0 - 2.6, 0.0) * sign(angSpeed);
	ang += angSpeed * f;
	
    vec4 color = texture2D(iChannel0, vec2(ang, t1 * 0.01));
    color +=     texture2D(iChannel0, vec2(ang + t1 * 0.01, t1 * 0.005)) * d;
    vec4 color2 = texture2D(iChannel1, vec2(d + t2 * 0.2, t2 * 0.01)) * d;
    color2 +=     texture2D(iChannel1, vec2(d + t2 * 0.18, t2 * 0.011));
	
	color = clamp(color * 10.0 - 7.0 + sin(iGlobalTime), 0.0, 1.0);
	color2 = clamp(color2 * 10.0 - 7.0 + d * 3.0 + cos(iGlobalTime * 0.67), 0.0, 1.0);
		
	color2 *= clamp((d - 0.4) * 100.0, 0.0, 1.0);
	color2 *= 1.0 - clamp((d - 0.7) * 100.0, 0.0, 1.0);
	vec4 color3 = vec4(clamp((d - 0.7) * 100.0, 0.0, 1.0)) * 
		          ceil(max(ff - 0.993, 0.0));
	vec4 color4 = vec4(clamp((d - 0.7) * 100.0, 0.0, 1.0)) * 
		          ceil(max(ff2 - 0.993, 0.0));
	
	gl_FragColor = min(color, color2) + color3 + color4;
}
