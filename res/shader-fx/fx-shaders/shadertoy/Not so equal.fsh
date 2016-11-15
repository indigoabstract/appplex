// https://www.shadertoy.com/view/XdSGDt
// Ensures two floats are equal, then divide both by the same number, then print the difference. At least in my videocard I can see a white vertical line.
void main(void)
{
	float nowaythiscanbeaconstant = (2014.0 - iDate.r);
	float f = 100.0;
	f += 0.5 + nowaythiscanbeaconstant;
	float x = gl_FragCoord.x;
	
	if (x == f){
		x /= iResolution.x;
		f /= iResolution.x;
		float d = abs(sign(x - f));
		gl_FragColor = vec4(d);
		}
	else
		gl_FragColor = vec4(0.0);
}
