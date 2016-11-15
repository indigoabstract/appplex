// https://www.shadertoy.com/view/lslSRj
// An attempt to replicate an old school plasma demo effect. Also, my first shader!
const float PI = 3.14159265;

float time = iGlobalTime *0.2;

void main(void)
{	
	float xSin1_mult = 120.0;
	float xSin2_mult = 142.3;
	
	float ySin1_mult = 90.0;
	float ySin2_mult = 150.3;

	float xVal1 = sin(gl_FragCoord.x / xSin1_mult + sin(time * 1.1) * 5.0);
	float xVal2 = sin(gl_FragCoord.x / xSin2_mult + sin(time / 1.2) * 6.0);
		
	float yVal1 = sin(gl_FragCoord.y / ySin1_mult + sin(time * 1.25) * 2.6);
	float yVal2 = sin(gl_FragCoord.y / ySin2_mult + sin(time / 1.05) * 5.5);
	
	float red   = (xVal1 + xVal2 + yVal1 + yVal2 + 1.0) / 4.0;
	float green = (xVal1 + xVal2 + yVal1 + yVal2 + 1.0) / 3.0;
	float blue  = 0.0 - (xVal1 + xVal2 + yVal1 + yVal2 - 1.0) / 4.0;

	gl_FragColor = vec4(red, green, blue, 0);
}
