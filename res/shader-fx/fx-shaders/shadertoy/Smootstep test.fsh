// https://www.shadertoy.com/view/MdfSD8
#define PI 3.14159265359
void main(void)
{
	vec2 uv = (gl_FragCoord.xy - 0.5*iResolution.xy) / 
		min (iResolution.x, iResolution.y);
	
	float r = length (uv);
	float R = 0.4;
	float M = 0.1 ;// (0.5 + 0.5*sin (iGlobalTime * PI));
	float x = uv.x + uv.y + 0.2*sin (iGlobalTime*PI/2.0);
		
	vec4 c1 = mix (
		vec4 (1.0, 1.0, 0.0, 0.0), 
		vec4 (0.0, 0.5, 1.0, 0.0), 
		smoothstep (R*(1.0 - M),R*(1.0 + M),r));
	
	vec4 c2 = mix (
		vec4 (1.0, 1.0, 0.0, 0.0),
		vec4 (0.0, 0.5, 1.0, 0.0),
		step (R,r));
		
	gl_FragColor = mix (
		vec4 (1.0, 0.0, 0.0, 0.0), 
		mix (c2, c1, step (0.0, x)),
		step (0.002, abs (x))) ;
}
