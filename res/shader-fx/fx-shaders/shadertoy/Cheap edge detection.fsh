// https://www.shadertoy.com/view/MsXXDH

// iChannel0: t4

float luma (vec3 rgb) {
	return (rgb.r + rgb.g + rgb.b)/3.0;
}

const float T = 0.04;
const float M = 1.0;
const float L = 0.002;

void main(void)
{
	vec2 uv = (gl_FragCoord.xy / iResolution.xy);
	vec4 tc = texture2D(iChannel0, uv);
	float l = luma (tc.rgb) ;
	
	float q0 = fwidth (l);
	float q1 = abs (dFdx (l)); 
	float q2 = abs (dFdy (l));

	vec4 ct = vec4 (1.0, 1.0, 1.0, 0.0);
	vec4 c0 = mix (ct, vec4 (1.0, 0.0, 0.0, 1.0), 
		smoothstep (T * (1.0 - M), T * (1.0 + M), q0));
	
	vec4 c1 = mix (ct, vec4 (0.0, 0.0, 1.0, 1.0), 				   
		smoothstep (T * (1.0 - M), T * (1.0 + M), q1));
	
	vec4 c2 = mix (ct, vec4 (0.0, 1.0, 0.0, 1.0),
		smoothstep (T * (1.0 - M), T * (1.0 + M), q2));
		
	
	float line0 = uv.x - 0.5;
	float line1 = uv.y - 0.5;
	
	vec4 cl = vec4 (1.0, 1.0, 0.0, 1.0);	
	vec4 cr = mix (cl, mix (c1, c2, step (0.0, line1)), step (L, abs (line1)));	
	vec4 cd = mix (cr, c0, step (0.0, line0));	
	vec4 cf = mix (cl, cd, step (L, abs(line0)));
	
	gl_FragColor = mix (tc, cf, cf.a);
		
		
	
}
