// https://www.shadertoy.com/view/Xs23D3

// iChannel2: kb

// Infinite multiscale Perlin noise. Based on Inigo Quilez's code (https://www.shadertoy.com/view/XslGRr). C key changes the colormap.
// --- Fractal noise simulating heterogeneous density in galactic clouds
// ---   -> help from Fabrice Neyret, https://www.shadertoy.com/user/FabriceNeyret2
// ---   -> noise functions from Inigo Quilez, https://www.shadertoy.com/view/XslGRr


// Number of computed scales
#define NbScales 22.

// Id of the lowest displayed scale (debug)
#define FirstScale 0.

// Anti aliasing
#define LimitDetails 2.5
#define SmoothZone 100.

// Manual Zoom / Auto Zoom
#define Anim 1

// Colormap
#define ClampLevel 1.

#define ZoomDistance 10.

// Size of the first Perlin Noise grid (debug)
#define FirstDivision 8.


// 0 : multiplicative
// 1 : additive
#define Mode 0

#define GazConcentration 0.

// Caracteristic ratio of the frequencies (0.5 for octaves)
#define fRatio .5

float t = iGlobalTime;

bool keyToggle(int ascii) {
	return !(texture2D(iChannel2,vec2((.5+float(ascii))/256.,0.75)).x > 0.);
}

// --- noise functions from https://www.shadertoy.com/view/XslGRr
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

vec2 hash( vec2 p ) {  						// rand in [-1,1]
	p = vec2( dot(p,vec2(127.1,311.7)),
			  dot(p,vec2(269.5,183.3)) );
	return -1. + 2.*fract(sin(p+20.)*53758.5453123);
}
float noise( in vec2 p ) {
    vec2 i = floor(p), f = fract(p);
	vec2 u = f*f*(3.-2.*f);
    return mix( mix( dot( hash( i + vec2(0.,0.) ), f - vec2(0.,0.) ), 
                     dot( hash( i + vec2(1.,0.) ), f - vec2(1.,0.) ), u.x),
                mix( dot( hash( i + vec2(0.,1.) ), f - vec2(0.,1.) ), 
                     dot( hash( i + vec2(1.,1.) ), f - vec2(1.,1.) ), u.x), u.y);
}


// -----------------------------------------------

vec3 colormap(float value) {
	float maxv = ClampLevel;
	vec3 c1,c2;
	float t;
	if (value < maxv / 3.) {
		c1 = vec3(1.);   	   c2 = vec3(1., 1., .5);
		t =  1./3.;
	} else if (value < maxv * 2. / 3.) {
		c1 = vec3(1., 1., .5); c2 = vec3(1., 0,  0.);
		t =  2./3. ;
	} else {
		c1 = vec3(1., 0., 0.); c2 = vec3(0.);
		t =  1.;
	}
	t = (t*maxv-value)/(maxv/3.);
	return t*c1 + (1.-t)*c2;
}

void main( void ) { // --------------------------------------
    vec2 uv = gl_FragCoord.xy/ iResolution.y;

	float d = 1.; // initial density
	
 #if Anim
	float cycle = cos(mod(-t,100.)/100.*2.*3.14);
	float n_tiles_level_1 = exp(cycle*cycle*ZoomDistance)*pow(2.,FirstDivision);
#else
	float n_tiles_level_1 = exp(iMouse.x/iResolution.x*ZoomDistance)*pow(2.,FirstDivision);
#endif
	
	// zoom and centering
	uv = (uv - vec2(.9,.5))*n_tiles_level_1 + vec2(.9,.5);
	
	float theta = 4.+.008*t; // some rotations, not necessary
    mat2 m = fRatio*mat2( cos(theta),sin(theta), 
					 -sin(theta),cos(theta) );
		
	// computation of the multiplicative noise
	float q = 1.;
	for (float i = 0.; i < NbScales; i++) {
		if (d<1e-2) continue;
		
		// multiply the amplitude to maintain the total density
		float c = (i+1.< NbScales) ? 2. : 1.;
		
		float nn = noise(uv + 10.7*i*i);

		for (float j = 0.; j < GazConcentration; j++) {
			nn = sin(nn*3.14159265359/2.);
		}
			
#if Mode == 0
		float n = c* 0.5*(1.+nn);
#else
		float n = nn;
#endif	
		
		// compute only the visible scales
		float crit = n_tiles_level_1 *q - iResolution.x/LimitDetails;
		if (crit < SmoothZone && i >= FirstScale) {
			if (crit>0.) {  // avoid aliasing
				float t = crit/SmoothZone;				
				n = n*(1.-t);
#if Mode == 0
				n += t;
#endif
			}
			
#if Mode == 0
			d *= n;
#else
			d += n*(0.5);
#endif			
			
			
		}
	
		uv = m*uv; q*= fRatio; // go to the next octave
	}
	
	d = clamp(d,0.0,d);
	gl_FragColor.xyz = (keyToggle(67)) ? vec3(exp(-d)) :colormap(exp(-d));
}
