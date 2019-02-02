// https://www.shadertoy.com/view/4dlGWn
#ifdef GL_ES
precision highp float;
#endif

#define ITER 128

void main(void)
{
	float t = iGlobalTime * 0.1;
	float sint = sin(t), cost = cos(t);
	
	vec2 c = (2.0 * gl_FragCoord.xy / iResolution.xy - 1.0)
			 * vec2(iResolution.x / iResolution.y, 1.0);

	c = vec2(c.x * cost + c.y * sint, c.y * cost - c.x * sint) 
		* abs(sint) - vec2(0.1,0.9);

	vec2 z = vec2(0.0);  

	for (int i = 0; i < ITER; ++i) {  
		z = vec2(z.x * z.x - z.y * z.y, 2.0 * z.x * z.y) + c;  

		if (dot(z,z) > 4.0) {
			float s = 0.125662 * float(i);
			gl_FragColor = vec4(vec3(cos(s+0.9),cos(s+0.3),cos(s+0.2)) * 0.4 + 0.6, 1.0);
			return;  
		}  
	}

	gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
