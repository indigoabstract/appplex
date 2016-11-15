// https://www.shadertoy.com/view/4dXSDN
#define R 0.002

void main(void)
{
	vec2 uv = (gl_FragCoord.xy - 0.5*iResolution.xy) / min(iResolution.x, iResolution.y);
	vec2 B1 = vec2(0.3, 0.3);
	vec2 B2 = vec2(0.8, 0.1);
	vec2 B3 = vec2(0.1, 0.7);
	
	float fi = iGlobalTime;
	
	mat2 rotation = mat2(
		cos(fi), -sin(fi),
		sin(fi), cos(fi)
	);
	
	vec2 sine = vec2(sin(iGlobalTime/1.6) - cos(iGlobalTime/1.6));
	
	B1 *= rotation * sine*sine;
	B2 *= rotation * sine;
	B3 *= rotation * 0.10 * sine;
	
	float dB1 = step(distance(uv.xy, B1), R);
	float dB2 = step(distance(uv.xy, B2), R);
	float dB3 = step(distance(uv.xy, B3), R);	
		
	vec4 color;
		
	color = mix(
		vec4(1.0, 1.0, 0.0, 1.0),
		vec4(0.1, 0.2, 0.2, 1.0),		
		smoothstep(
			0.02,
			0.052,
			distance(uv.xy, B1) * distance(uv.xy, B2) * distance(uv.xy, B3)
		)
	);
	
	gl_FragColor = color;
}
