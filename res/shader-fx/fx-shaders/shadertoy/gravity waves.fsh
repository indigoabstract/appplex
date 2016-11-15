// https://www.shadertoy.com/view/lsBGWd

// iChannel0: m0

// This is a (fictional) visual of B-mode polarization, indirect evidence of gravity waves. Inspired by the pic at http://www.theregister.co.uk/2014/03/17/gravity_waves_big_bang_bicep2/
/* (Fictional) visual of B-mode polarization, indirect evidence of "primordial" gravity waves.

  Inspired by http://bicepkeck.org/visuals.html
 "Gravitational waves from inflation generate a faint but distinctive twisting pattern in
 the polarization of the CMB, known as a "curl" or B-mode pattern. For the density
 fluctuations that generate most of the polarization of the CMB, this part of the primordial
 pattern is exactly zero. Shown here is the actual B-mode pattern observed with the BICEP2
 telescope, with the line segments showing the polarization from different spots on the sky.
 The red and blue shading shows the degree of clockwise and anti-clockwise twisting of this
 B-mode pattern."

 For explanatory info, see:
 http://wwwphy.princeton.edu/cosmology/capmap/polscience.html
 http://physics.stackexchange.com/questions/103934/what-do-the-line-segments-on-the-bicep2-b-mode-polarization-map-mean

*/

/* Overall plan:
 x generate scalar noise f(x, y, t) for each point on the plane, varying over time.
 x for each pixel, color red for positive or blue for negative, white in the middle.
   => bgcolor
 - for each pixel, find out what grid point we're nearest.
   - find the degree of twisting for that grid point (a separate noise function?)
     g(x, y, t) => (m, a) magnitude and angle
     or maybe that's two separate functions. Anyway...
   - compute the line segment(rotated rectangle) representing the twisting.
   - decide whether current pixel is inside, outside, or on the edge of that rectangle.
   - mix bgcolor and black accordingly.
     
*/


//
// Description : Array and textureless GLSL 2D/3D/4D simplex
// noise functions.
// Author : Ian McEwan, Ashima Arts.
// Maintainer : ijm
// Lastmod : 20110822 (ijm)
// License : Copyright (C) 2011 Ashima Arts. All rights reserved.
// Distributed under the MIT License. See LICENSE file.
// https://github.com/ashima/webgl-noise
//

vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  {
  const vec2 C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i = floor(v + dot(v, C.yyy) );
  vec3 x0 = v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  // x0 = x0 - 0.0 + 0.0 * C.xxx;
  // x1 = x0 - i1 + 1.0 * C.xxx;
  // x2 = x0 - i2 + 2.0 * C.xxx;
  // x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i);
  vec4 p = permute( permute( permute(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3 ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z); // mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ ); // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }


const float cellSize = 0.14; // size of each grid cell as portion of viewport
const float scale = 1.5;
float time = iGlobalTime * 0.1;
const float lineThick = 0.01, borderThick = 0.005;
const int nSamples = 20;

float sampleVolume() {
	float total = 0.0;
	for (int i=0; i < nSamples; i++) {
		total += texture2D(iChannel0, vec2(float(i) * 0.5 / float(nSamples), 0.25)).x;
	}
	return total / float(nSamples);
}

// Generate red/white/blue value representing "twist" of B-mode polarization.
// TODO: wish I could get more of the "loopy" shape that the real thing has.
// Could do that using a sort of ripples-from-several-charges effect.
vec4 twistColor(vec2 p) {
	float intens = snoise(vec3(p.x, p.y, time)) * 0.9;
	// Modify the shape of the curve, keeping it closer to zero longer:
	// intens *= intens * intens * 8.0;
	float i2 = pow(intens, 1.7) * 1.7;

	if (iChannelTime[0] > 32.0) {
		float sv = sampleVolume();
		i2 *= (sv * sv + 1.0);
	}
	
	vec4 color;
	if (intens > 0.0) {
		color = vec4(1.0, 1.0 - i2, 1.0 - i2, 1.0);
	} else {
		color = vec4(1.0 - i2, 1.0 - i2, 1.0, 1.0);
	}
	return color;
}

/*
// antialias coefficient
#define aa(d) ((linethick - (d)) / linethick)
*/
float aa(float d) {
	return (lineThick - d) / lineThick;
}

// Compute whether p is inside, outside, or on the edge of
// a rotated rectangle representing blah blah ...
// Where <= 0 is inside, >= 1 is outside.
float barDist(vec2 p) {
	// Find center of grid cell.
	vec2 c = floor(p / cellSize + 0.5) * cellSize;

	// generate amplitude and angle
	float amp = abs(snoise(vec3(c.xy, time))) * 0.125;
	float alpha = snoise(vec3(c.xy, time)) * 3.14159;
	
	//## Here is where I need to write the code for the rotated rectangle
	// and its distance test.
	
	float d = distance(p, c);
	if (d > amp) return 1.0;
	// angle to current pixel from center of circle
	float a = atan(p.y - c.y, p.x - c.x);
	// dist from p to line
	float dd = abs(sin(alpha - a) * d);
	if (dd < lineThick) {
		return dd/lineThick;
	}
	
	return 1.0;
	
}

const vec4 black = vec4(0.0);

void main(void)
{
	vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 p = (-1.0 + 2.0*q) * scale;
    p.x *= iResolution.x / iResolution.y;

	vec4 bgColor = twistColor(p);
	
	float d = barDist(p);
	gl_FragColor = mix(black, bgColor, d);
	
}
