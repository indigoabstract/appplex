// https://www.shadertoy.com/view/4dsXRX
//#define CONST_SIZE
#ifdef CONST_SIZE
#define lines 5.0
#define gradLength (1.0 / lines)
#endif
#define PI 3.14159

void main(void){
	
#ifndef CONST_SIZE
	float lines = sin(iGlobalTime / 6.0 + 4.0) * 20.0 + 22.0;
	float gradLength = (1.0 / lines);
#endif
	
	vec2 uv = gl_FragCoord.xy;
	vec2 center = iResolution.xy * 0.5;
	vec2 delta = uv - center;
	delta.x = abs(delta.x);
	
	float len = length(delta);
	float gradStep = floor(len * 0.005 * lines) / lines;
	float gradSmooth = len * 0.005;
	float gradCenter = gradStep + (gradLength * 0.5);
	float percentFromCenter = abs(gradSmooth - gradCenter) / (gradLength * 0.5);
	float interpLength = 0.01 * lines;
	float s = 1.0 - smoothstep(0.5 - interpLength, 0.5 + interpLength, percentFromCenter);
	
	float index = gradStep / gradLength;
	vec4 color = vec4(sin(index*0.55), sin(index*0.2), sin(index*0.3), 1)*0.5 + vec4(0.5, 0.5, 0.5, 0);
	
	float angle = atan(delta.x, delta.y);
	float worldAngle = sin(gradStep * 4.0 + iGlobalTime) * PI * 0.5 + PI * 0.5;
	
	if(angle < worldAngle){
		gl_FragColor = vec4(1,1,1,1.0);	
		vec2 tip = vec2(sin(worldAngle), cos(worldAngle)) * gradCenter * 200.0;
		
		float tipDist = length(delta - tip);
		
		float rad = 50.0 / lines;
		float tipC = 1.0 - smoothstep(rad - 1.0, rad + 1.0, tipDist);
		
		gl_FragColor = vec4(tipC,tipC,tipC,1);	
	}else{
		gl_FragColor = vec4(s,s,s,1);
	}
	gl_FragColor *= color;
}
