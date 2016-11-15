// https://www.shadertoy.com/view/ldjGDt
//20/03/2014 - Click on the shader to activate a different cardioid shape
//26/03/2014 - Added a basic border smoothing, enable it with the SMOOTH define

#define SMOOTH true

float s_mask(vec2 p, float s) {
	float fw = fwidth(s);
	return smoothstep(-fw, fw, s);
}

void main(void)
{
	vec2 p = (-1.0 + 2.0*gl_FragCoord.xy / iResolution.xy)*vec2(iResolution.x/iResolution.y, 1.0);
	float beat = abs(sin(iGlobalTime)) + 0.7;
	float x = beat*p.x*3.0;
	float y = beat*p.y*4.0;
	float b = 1.2;
	float shape = x*x + pow((y - pow(x*x, 1.0/3.0)), 2.0) - 1.5;
	//float shape = pow(x*x+y*y-1.0, 3.0) - (x*x*y*y*y); //smooth not working well
	
	vec4 bCol = vec4(1.0, 0.65, 0.74, 1.0)*vec4(vec3(1.0 - 0.3*length(p)), 1.0);
	vec4 sCol = vec4(0.7, 0.062, 0.23, 1.0);
	
	if(iMouse.z > 0.0) {
		y -= 1.0;
		shape = pow(x*x+y*y+b*y, 2.0) - b*b*(x*x+y*y);
	}
	
	vec4 col = (shape < 0.0) ? sCol : bCol;
	
	if(SMOOTH) {
		float m = s_mask(p, shape);
		col = mix(sCol, bCol, m);
	}
	
	gl_FragColor = col;
}
