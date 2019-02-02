// https://www.shadertoy.com/view/Msf3R8
//#define N_IT       1025600.0
#define N_IT       256.0
#define R_MAX      5000.0
#define PI         3.1415926535
#define FACT       1000.0

float time = iGlobalTime;

float getIterations(in vec2 c) {
	float it = 0.0;
	vec2 z = vec2(0.0);
	for (float i=0.0; i<N_IT; i++) {
		if (length(z) < R_MAX*FACT) {
			z = vec2(z.x*z.x - z.y*z.y, 2.0*z.x*z.y)/FACT + c;
			it += 1.0;
		}
	}
    it += 1.0 - log2(0.7*log2(length(z)/FACT));
	return sqrt(it/70.0);
}

vec4 calcFractColor(in vec2 p) {
	float r = 2.0*PI*getIterations(p);
	float f = PI*2.0/6.0;
	return 0.3+0.7*vec4(cos(r+2.0*f), cos(r+f), cos(r), 1.0);
}

void main(void) {
	vec2 center = vec2(0.444455, 0.375305);
	//vec2 center = vec2(-1.37298, 0.085733);
	float zoom = 0.000002*FACT+ exp(-mod(iGlobalTime, 17.5)/1.0+log(FACT));
	//float zoom = 0.000002*FACT+ exp(-iGlobalTime/1.0+log(FACT));
	
	vec2 p = -1.1+2.2*gl_FragCoord.xy / iResolution.yy;
	p += vec2(-1.1,0.0);
	p -= center;
	float rCenter = length(p);
	p *= zoom;
	p += center*FACT;
	float r = length(p)/FACT, f;
	vec4 col = vec4(0.0, 0.0, 0.0, 1.0);
	
	col += calcFractColor(p);
	
	f = 1.0-0.2*smoothstep(1.0, 1.01, r)*(1.0-smoothstep(1.01, 1.02, r));
	col *= f;
	
	float rat = 0.01;
	f = 0.2+0.8*smoothstep(rat, rat*1.01, rCenter);
	col *= f;
	
	gl_FragColor = col;
}
