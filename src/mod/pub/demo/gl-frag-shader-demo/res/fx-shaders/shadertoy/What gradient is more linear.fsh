// https://www.shadertoy.com/view/XslXRr
// Click to see the solution (actual midtone)
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float col = 0.0;
	bool solution = iMouse.z > 0.0;

	if (uv.y < (solution ? 0.33 : 0.5))
		col = sqrt(uv.x);
	
	else if(uv.y < 0.66 && solution)
		col = step(0.5, fract((gl_FragCoord.x + gl_FragCoord.y) / 2.0));
	else
		col = uv.x;
	float p = 1.0 - 0.5*step(0.66, uv.y);
	gl_FragColor = vec4(col, col, 0.0, 1.0);
}
