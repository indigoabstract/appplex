// https://www.shadertoy.com/view/XsfGz4
// View IQ's clouds demo, through a letter-pi-shaped viewport.
// For Pi Day, Mar. 14, 2013
// Pi is illustrated by the ratio of the blue line
// to the red line.

// Uses techniques similar to Ika-chan (https://www.shadertoy.com/view/lssGRn)
// except not as sophisticated, for outlining the shape of the letter pi.
									  
// To get the pi shape, could use the svg path from
// http://commons.wikimedia.org/wiki/File:Greek_letter_pi_serif%2Bsans.svg
// (use the second part of the first path, after the z M).
// But that would require some complex hit-testing code.
// Maybe I can just do it with rectangles and circles
// (positive and negative).

// TODO: AA the edges of the letter pi. Make the hit testing functions return a float.

// TODO: make the outline fancier

	
// Cloud demo created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

vec2 p;

mat3 m = mat3( 0.00,  0.80,  0.60,
              -0.80,  0.36, -0.48,
              -0.60, -0.48,  0.64 );

float hash( float n ) {
    return fract(sin(n)*43758.5453);
}

float noise( in vec3 x ) {
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return res;
}

float fbm( vec3 p ) {
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p );
    return f;
}

vec4 map( in vec3 p ) {
	float d = 0.2 - p.y;

	d += 3.0 * fbm( p*1.0 - vec3(1.0,0.1,0.0)*iGlobalTime );

	d = clamp( d, 0.0, 1.0 );
	
	vec4 res = vec4( d );

	res.xyz = mix( 1.15*vec3(1.0,0.95,0.8), vec3(0.7,0.7,0.7), res.x );
	
	return res;
}


vec3 sundir = vec3(-1.0,0.0,0.0);


vec4 raymarch( in vec3 ro, in vec3 rd ) {
	vec4 sum = vec4(0, 0, 0, 0);

	float t = 0.0;
	for(int i=0; i<44; i++)
	{
		vec3 pos = ro + t*rd;
		vec4 col = map( pos );
		
		#if 1
		float dif =  clamp((col.w - map(pos+0.3*sundir).w)/0.6, 0.0, 1.0 );

        vec3 brdf = vec3(0.65,0.68,0.7)*1.35 + 0.45*vec3(0.7, 0.5, 0.3)*dif;
		col.xyz *= brdf;
		#endif
		
		col.a *= 0.35;
		col.rgb *= col.a;

		sum = sum + col*(1.0 - sum.a);	

		//if (sum.a > 0.99) break;
        #if 0
		t += 0.1;
		#else
		t += max(0.1,0.05*t);
		#endif
	}

	sum.xyz /= (0.001+sum.w);

	return clamp( sum, 0.0, 1.0 );
}


bool inCircle(vec2 c, float r) {
	return (distance(p, c) <= r);
}

// c1 is lower-left corner of rectangle;
// c2 is upper-right corner.
bool inRect(vec2 c1, vec2 c2) {
	return (p.x >= c1.x && p.y >= c1.y &&
		p.x <= c2.x && p.y <= c2.y);
}

const float bot = -0.6, top = 0.8, left = -0.7, right = 0.65;

bool inViewport() {
	return (
		// top bar
		inRect(vec2(left, 0.4), vec2(right, top)) ||
		// top left quarter circle
		(inCircle(vec2(left, 0.4) , 0.4) && p.y > 0.4) ||
		// top right quarter circle
		(inCircle(vec2(right, 0.8), 0.4) && p.y < top) ||
		// left leg
		inRect(vec2(-0.65, bot), vec2(-0.25, 0.4)) ||
		// right leg
		inRect(vec2(0.25, bot), vec2(0.65, 0.4)) ||
		inCircle(vec2(-0.5, bot), 0.25) ||
		inCircle(vec2( 0.5, bot), 0.25) 
	);
}

const float r = 0.25;
const float linethick = 0.015;
const float linetop = top - r, mid = (linetop + bot) * 0.5;

const vec2 start = vec2(left - r - 0.45, bot);
const vec3 circumcol = vec3(0.8, 0.2, 0.2), gray = vec3(0.25),
	blue = vec3(0.4, 0.4, 0.8);

const float pi = 3.1415926535897932384626433; // Hooray!

/*
// antialias coefficient
#define aa(d) ((linethick - (d)) / linethick)
*/
float aa(float d) {
	return (linethick - d) / linethick;
}

vec3 piDemo(vec2 p) {
	float d; // distance from various things
	bool onLine = false;
	float t = sin(iGlobalTime); // -1 to 1
	t = sign(t) * sqrt(abs(t)); // spend more time near the ends
	
	// center of rolling circle
	vec2 c = vec2(start.x - r, t * (linetop - bot) * 0.5 + mid);
	
	// vertical line - red
	d = abs(p.x - start.x);
	if (d < linethick && p.y > start.y && p.y < top - r) {
		onLine = true;
		// above circle?
		if (p.y > c.y)
			return circumcol * aa(d);
	}

	// length of "string" below circle's tangent point
	float len = (t + 1.0) * 0.5; // map t to 0..1 range
	
	// rolling circle
	d = distance(p, c);
	if (d < r + linethick) {
		// angle to current pixel from center of circle
		float a = atan(p.y - c.y, p.x - c.x);
		// put into range 0 to 2pi
		if (a < 0.0) a += pi * 2.0;
		// angle interval that string wraps to
		float wrapa = (1.0 - len) * 2.0 * pi;
		
		// on circumference?
		if (abs(d - r) < linethick) {
			if (a > wrapa)
				return circumcol * aa(abs(d - r));
			else return gray * aa(abs(d - r));
		}
		
		// on diameter?
		if (d < r) {
			float dd = abs(sin(wrapa - a) * d);
			if (dd < linethick) {
				return blue * aa(dd);
			} else {
				// shouldn't be necessary
				return vec3(0.0);
			}

		}
	}

	// vertical line, gray below circle
	if (onLine) {
		return gray * aa(abs(p.x - start.x));
	}

	return vec3(0.0);
}

void main(void)
{
	vec2 q = gl_FragCoord.xy / iResolution.xy;
    p = -1.0 + 2.0*q;
    p.x *= iResolution.x/ iResolution.y;
	p.x -= 0.3;

	if (!inViewport()) {
		gl_FragColor = vec4(piDemo(p).xyz, 1.0);
		return;
	}
		
    vec2 mo = -1.0 + 2.0 * iMouse.xy / iResolution.xy;
    
    // camera
    vec3 ro = 4.0*normalize(vec3(cos(2.75-3.0*mo.x), 0.7+(mo.y+1.0), sin(2.75-3.0*mo.x)));
	vec3 ta = vec3(0.0, 1.0, 0.0);
    vec3 ww = normalize( ta - ro);
    vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
    vec3 vv = normalize(cross(ww,uu));
    vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );

	
    vec4 res = raymarch( ro, rd );

	float sun = clamp( dot(sundir,rd), 0.0, 1.0 );
	vec3 col = vec3(0.6,0.71,0.75) - rd.y*0.2*vec3(1.0,0.5,1.0) + 0.15*0.5;
	col += 0.2*vec3(1.0,.6,0.1)*pow( sun, 8.0 );
	col *= 0.95;
	col = mix( col, res.xyz, res.w );
	col += 0.1*vec3(1.0,0.4,0.2)*pow( sun, 3.0 );
	    
    gl_FragColor = vec4( col, 1.0 );
}
