// https://www.shadertoy.com/view/4dfXzH
// Modified Perlin noise with uniform distribution of values. Move the mouse to add scales. Left : modified Perlin noise - Right : Perlin noise Top : multiplicative noise - Bottom : additive noise
// --- Normalized Perlin's Noise (left)
// The color of the pixels of the Perlin's noise (right) follow a normal distribution. Its histogram is a 
// Gaussian bell curve : lots of pixels have a value near 0, and very few are near -1 or 1. 
// This shader produces a normalized Perlin's noise. The values are uniformly distributed between -1
// and 1. With this noise, we are able to simulate other noises with different inverse distribution 
// functions.

// Approximation of the standard deviation of Perlin's noise
#define sigma .175 
#define size 12.

// --- noise functions from https://www.shadertoy.com/view/XdXGW8
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

vec2 hash( vec2 p ) {  						// rand in [-1,1]
	p = vec2( dot(p,vec2(127.1,311.7)),
			  dot(p,vec2(269.5,183.3)) );
	return -1. + 2.*fract(sin(p+3.)*53758.5453123);
}

float noise( in vec2 p ) {
    vec2 i = floor(p), f = fract(p);
	vec2 u = f*f*f*(6.*f*f - 15.*f + 10.);
;
    return mix( mix( dot( hash( i + vec2(0.,0.) ), f - vec2(0.,0.) ), 
                     dot( hash( i + vec2(1.,0.) ), f - vec2(1.,0.) ), u.x),
                mix( dot( hash( i + vec2(0.,1.) ), f - vec2(0.,1.) ), 
                     dot( hash( i + vec2(1.,1.) ), f - vec2(1.,1.) ), u.x), u.y);
}

// Error function approximation for x > 0
float erf(float x) {
	float a1 =  0.278393;
	float a2 =  0.230389;
	float a3 =  0.000972;
	float a4 =  0.078108;
	float denom = 1. + (a1 + (a2+(a3+a4*x)*x)*x)*x;
	return 1.-1./(pow(denom,4.));
}

float nbscales = floor((iMouse.x/iResolution.x)*5.) +1.;

void main( void ) {
	vec2 p = gl_FragCoord.xy / iResolution.xy;
	vec2 uv = p*vec2(iResolution.x/iResolution.y,1.0);
	float res = 1.;
	float angle = 2.;
	mat2 m = 2.*mat2(cos(angle),sin(angle),-sin(angle),cos(angle));
	float ampl = 1.;
	
	for (float i = 0.; i < 5.; i++) {
		if (i < nbscales) {
			
			// Perlin's noise
			float pN = noise(uv*size);
			
			// Multiplicative
			if (p.x<0.6 && p.y > 0.5) {
				// Normalized noise
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res *= 1.+nN;
			} else if (p.x>0.605  && p.y > 0.5) {
				res *= 1.+pN;
				
			// Additive
			} else if (p.x<0.6 && p.y < 0.49) {
				// Normalized noise
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res += nN*ampl;
			} else if (p.x>0.605 && p.y < 0.49){
				res += pN*ampl;
			}
			uv = m*uv;
			ampl /= 2.;
		}
	}
	gl_FragColor.xyz =  vec3(res/2.);
}
