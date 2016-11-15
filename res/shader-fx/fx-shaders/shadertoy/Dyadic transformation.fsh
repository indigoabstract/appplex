// https://www.shadertoy.com/view/XsX3Dl
// An implmentation of Dyadic transformation, also known as the Bit shift map. http://en.wikipedia.org/wiki/Bit_shift_map
float dyadicIteration(float x) {
	return x < 0.5 ? 2.*x : 2.*x-1.;	
}

#define ITERATIONS 20

void main(void)
{
	vec2 v = (gl_FragCoord.xy-0.5*iResolution.xy) / iResolution.x;
	float x = length(v);
	float alpha = sin(iGlobalTime)*0.40+0.5;
	float c = 0.0;
	for(int i = 0; i < ITERATIONS; ++i) {
		x = dyadicIteration(x);
		c = alpha*c+(1.0-alpha)*x;
	}
	
	gl_FragColor = vec4(vec3(c),1.0);
}
