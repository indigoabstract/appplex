// https://www.shadertoy.com/view/4sfGWr

// iChannel0: t11

/* Basic idea:
   Show a surface of voronoi shapes. We generate n centroids, and
	vary their positions over time, using noise and/or fbm and/or just sine functions.
   Then for each uv, we find the nearest centroid, and render the point using diffuse &
   specular reflection.
   The normal could be computed maybe as if on a sphere whose center is the centroid.

   Bonus points for iridescent coloring. :-)
   Or probably color each sphere a stable color, maybe based on its index.
	But it could have iridescence on top.
   Also, could reflect a cubemap.
   
   iChannel0 supplies the colors for the various regions.
   I've used the texture that looks like colored static, but others
   are good too.

   Also try weighted voronoi diagrams - multiplicatively or additively
   (for circular or hyperbolic edges).
   It's easy to modify the d formula; the trouble comes in adjusting the
   drawing of centroid dots and borders to match.

   Another option: shading by distance from nearest centroid.
 */


const float PI = 3.14159265;
const float centroidDotRadius = 0.02;
const float lineThickness = 0.02;

// Uncomment to use FBM: more random motion of centroids, but slower framerate.
// #define USE_FBM 1 // not working at present

// rand, noise, and fbm copied from https://www.shadertoy.com/view/Xsl3zN "Fire" by @301z

// Given a vec2 seed n, return a pseudorandom float between 0 and 1.
float rand(vec2 n) { 
	return fract(sin(dot(n, vec2(12.9898, 4.1414))) * 43758.5453);
}

#ifdef USE_FBM
// Seems to be pretty fast. Should return a number between 0 and 1.
float noise(vec2 n) {
	const vec2 d = vec2(0.0, 1.0);
	vec2 b = floor(n), f = smoothstep(vec2(0.0), vec2(1.0), fract(n));
	return mix(mix(rand(b), rand(b + d.yx), f.x), mix(rand(b + d.xy), rand(b + d.yy), f.x), f.y);
}

// Fractional brownian motion. Should return a number between 0 and 2,
// with the highest probabilities nearest 1 (gaussian distribution?).
float fbm(vec2 n) {
	float total = 0.0, amplitude = 1.0;
	for (int i = 0; i < 7; i++) {
		total += noise(n) * amplitude;
		n += n;
		amplitude *= 0.5;
	}
	return total;
}
#endif

const int nRegions = 16;

// No point in storing them:
// struct region {	vec2 c; } regions[nRegions];

void main(void)
{
	vec2 uv = (gl_FragCoord.xy * 2.0 - iResolution.xy) / iResolution.yy;

	vec4 col = vec4(0.0, 0.0, 0.0, 1.0);

	int closestI = -1;
	float closestD = 9998.0, nextClosestD = 9999.0;
	vec2 closestC, nextClosestC;
	bool isDot = false;
	
	// place region centroids, and find nearest
	for (int i=0; i < nRegions; i++) {
		// convert i to a float between -1 and 1:
		float fi = (2.0 * float(i) / float(nRegions) - 1.0);

		float t = (iGlobalTime + 200.0) * (0.2 + fi * 0.3) * 0.2;

		// to do: add in noise or fbm to placement.
		// TODO: use i. Maybe a spiral path.
		float a1 = (2.0 * PI) * (t * 0.2 + 0.1);
		float r1 = 0.7 + 0.5 * sin(t * fi * 0.2);
		float a2 = t * 1.67 + 3.8;
		float r2 = 0.4 + 0.7 * sin(t * (0.6 + fi * 0.27));
#ifdef USE_FBM
		float xoff = fbm(vec2(t + fi, t + fi + 1.0)) - 0.5;
		float yoff = fbm(vec2(t - fi, t - fi + 1.0)) - 0.5;
#else
		float xoff = 0.0, yoff = 0.0;
#endif
		// Assign this to regions[i].c if needed.
		vec2 c = vec2(cos(a1) * r1 + cos(a2) * r2 + xoff * 0.5,
					  sin(a1) * r1 + sin(a2) * r2 + yoff * 0.5);
		// vec2(sin(t * 2.0 + 0.5), cos(t * 2.5));
		// weighting
		float w = 2.0 + fi;

		if (!isDot) {
			float d = distance(c, uv); // distance to centroid // *w or -w
			if (d < closestD) {
				closestI = i;
				nextClosestD = closestD;
				closestD = d;
				nextClosestC = closestC;
				closestC = c;
				if (d < centroidDotRadius) {
					isDot = true;
				}
			} else if (d < nextClosestD) {
				nextClosestD = d;
				nextClosestC = c;
			}
		}		
	}
	
	closestD = max(closestD, 0.00001); // avoid /0 errors

	// Get color of region i from texture.
	float rr1 = rand(vec2(float(closestI), 1.0));
	float rr2 = rand(vec2(float(closestI + 1), 1.0));
	col = mix(texture2D(iChannel0, vec2(rr1, rr2)), vec4(1.0), 0.5);
	
	if (isDot) {
		col *= smoothstep(centroidDotRadius * 0.5,
						  centroidDotRadius * 1.5, closestD);
	} else {
		// Shade according to distance from nearest centroid
		// TODO: make this dropoff bell-shaped. The gaussian formula at
		// http://en.wikipedia.org/wiki/Normal_distribution looks complicated, but
		// maybe that's the best answer. Use mu=0, sigma^2=5?
		// What about http://en.wikipedia.org/wiki/Normal_distribution#Standard_normal_distribution ?
		// That seems to imply mu=0, sigma=1
		// Here is the standard normal distribution, with the constant coefficient (1 / sqrt(2 pi)) removed.
		// That puts it more in the range we want, anyway. I think.
		const float sigma = 0.8; // Greater sigma makes bell curve less steep.
		col *= 1.2 * exp(closestD * closestD * (-0.5 / (sigma * sigma)));
		
		// Compute distance from edge of region, so we can draw border.
		// midpoint between two closes centroids:
		vec2 c3 = (nextClosestC + closestC) * 0.5;
		vec2 pc3 = c3 - uv;
		vec2 c1c2 = nextClosestC - closestC;
		float fromEdge = dot(normalize(c1c2), pc3);
		
		if (fromEdge <= lineThickness) {
			// TODO: why are there weird kinks at the corners?
			col *= smoothstep(lineThickness * 0.25, lineThickness * 0.75, fromEdge);
		}
	}

	gl_FragColor = col;
}
