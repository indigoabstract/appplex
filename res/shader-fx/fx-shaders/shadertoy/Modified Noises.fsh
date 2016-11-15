// https://www.shadertoy.com/view/MsfSz8
// Modified Perlin noises with control of the distribution functions. Moving the mouse add low frequencies. Perlin Noise | Normalized noise | Cauchi | Exponential | 2 Peaks See https://www.shadertoy.com/view/4dfXzH
// --- Noises
// See https://www.shadertoy.com/view/4dfXzH for explanations

// Approximation of the standard deviation of Perlin's noise

#define sigma .175 
#define size 12.
#define PI 3.14159265359

// --- noise functions from https://www.shadertoy.com/view/XdXGW8 
//     & https://www.shadertoy.com/view/lsf3WH
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
/*

float hash( vec2 p )
{
	float h = dot(p,vec2(127.1,311.7));
	
    return -1.0 + 2.0*fract(sin(h)*43758.5453123);
}

float noise( in vec2 p )
{
    vec2 i = floor( p );
    vec2 f = fract( p );
	
	vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}
*/

// Error function approximation for x > 0
float erf(float x) {
	float a1 =  0.278393;
	float a2 =  0.230389;
	float a3 =  0.000972;
	float a4 =  0.078108;
	float denom = 1. + (a1 + (a2+(a3+a4*x)*x)*x)*x;
	return 1.-1./(pow(denom,4.));
}

float erfinv(float x) {
	float a = 0.147;
	return x/abs(x)*sqrt(sqrt((2./(PI*a)+log(1.-x*x)/a)-log(1.-x*x)/a)-2./(PI*a)+log(1.-x*x)/2.);
}

float cauchiDistribution(float x) {
	float a = 0.09;
	float cauchi = tan(PI*(x-1./2.))*a;
	return clamp(cauchi, -1., 1.);
}

float exponentialDistribution(float x) {
	float lambda = 2.;
	return -1.-log((x+1.)/2.)/lambda;
}

// histogram gives more or less the sum of two gaussian curves
float peaks(float x) {
	float res;	
	if (x <= 0.) {
		res = tan(PI*(x+0.5)/2.)*0.5-0.5;
	} else {
		res = tan(PI*(x-0.5)/2.)*0.5+0.5;
	}
	return res;
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
			if (p.x<0.2 && p.y > 0.5) {
				res *= 1.+pN;
			} else if (p.x<0.4  && p.y > 0.5) {
				// Normalized noise
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res *= 1.+nN;
			} else if (p.x<0.6  && p.y > 0.5) {
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res *= 1.+cauchiDistribution(nN/2.+0.5);
			} else if (p.x<0.8  && p.y > 0.5) {
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res *= 1.+exponentialDistribution(nN);
			} else if (p.y > 0.5) {
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res *= 1.+peaks(nN);
				
			// Additive
			} else if (p.x<0.2 && p.y < 0.49) {
				res += pN*ampl;
			} else if (p.x<0.4 && p.y < 0.49){
				// Normalized noise
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res += nN*ampl;
			} else if (p.x<0.6 && p.y < 0.49){
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res += cauchiDistribution(nN/2.+0.5)*ampl;
			} else if (p.x<0.8 && p.y < 0.49){
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res += exponentialDistribution(nN)*ampl;
			} else if (p.y < 0.49){
				float nN = (pN > 0.) ? erf(pN/sqrt(2.)/sigma) : -erf(-pN/sqrt(2.)/sigma);
				res += peaks(nN)*ampl;
			}
			uv = m*uv;
			ampl /= 2.;
		}
	}
	gl_FragColor.xyz =  vec3(res/2.);
}
