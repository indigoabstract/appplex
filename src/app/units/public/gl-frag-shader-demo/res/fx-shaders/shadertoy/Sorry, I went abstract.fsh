// https://www.shadertoy.com/view/XsXGD8

// iChannel0: t12

#define TIME iGlobalTime

mat2 rotate(float a) { return mat2(cos(a),-sin(a),sin(a),cos(a)); }

float noise(float x) { return texture2D(iChannel0,vec2(x)).x;}

#define WEBN 200.

void main(void) {
	vec2 p = (2.0*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
	float l = 0.0;
	float mz = 0.0;
		
	for (float i=0.; i < WEBN; i+=1.0) {
		float fi = i/WEBN*acos(-1.)*2.0;
		float n = noise(fi);
		
		float z = 1.0-mod(TIME/2.0+i/WEBN,1.);	
		vec2 o = p;
		o -= vec2(sin(2.5*fi+TIME)*2.0,cos(2.0*fi+TIME))*z;
		o *= rotate(TIME*n*2.0)/(z*15.);
		l += (smoothstep(0.98,1.0,sin(mod(length(o),0.5)*30.0)))*(0.5-z);
		mz = min(mz,z);
	}
	
	vec3 col = mix(vec3(0.2,0.0,0.1),vec3(0.9,1.0,1.0),l*(0.25+mz));
	gl_FragColor = vec4(col,1.0);
}
