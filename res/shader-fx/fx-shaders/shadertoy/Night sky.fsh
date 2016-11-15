// https://www.shadertoy.com/view/4sfGRB

// iChannel0: t12

/* Created by Nikita Miropolskiy, nikat/2013
 * Creative Commons Attribution-NonCommercial-NoDerivs 3.0 Unported License.
 * http://creativecommons.org/licenses/by-nc-nd/3.0/
 */

//#define PROCEDURAL_NOISE

vec2 rotate2(vec2 p, float theta) {
	return vec2(
		p.x*cos(theta)-p.y*sin(theta),
		p.x*sin(theta)+p.y*cos(theta)
	);
}


/* discontinuous random vec2 */
vec2 random2(vec2 c) {
	float j = 4906.0*sin(dot(c,vec2(169.7, 5.8)));
	vec2 r;
	r.x = fract(512.0*j);
	j *= .125;
	r.y = fract(512.0*j);
	return r-0.5;
}

#ifdef PROCEDURAL_NOISE

/* skew constants for 2d simplex functions */
const float F2 =  0.3660254;
const float G2 = -0.2113249;

/* 2d simplex (tetrahedron) derivative noise*/
float simplex2d(vec2 p) {
	 vec2 s = floor(p + (p.x+p.y)*F2);
	 vec2 x = p - s - (s.x+s.y)*G2;
	 
     float e = step(0.0, x.x-x.y);
	 vec2 i1 = vec2(e, 1.0-e);
	 	 
	 vec2 x1 = x - i1 - G2;
	 vec2 x2 = x - 1.0 - 2.0*G2;
	 
	 vec3 w, d;
	 	 
	 w.x = dot(x, x);
	 w.y = dot(x1, x1);
	 w.z = dot(x2, x2);
	 	 
	 w = max(0.5 - w, 0.0);
	 
	 d.x = dot(random2(s + 0.0), x);
	 d.y = dot(random2(s +  i1), x1);
	 d.z = dot(random2(s + 1.0), x2);
	 
	 w *= w;
	 w *= w;
	 d *= w;
	 	 
	 return 0.5+dot(d, vec3(70.0));
}

#else

float simplex2d(vec2 p) {
	return texture2D(iChannel0, p*0.015).x;
}

#endif


/* random star parameters */
vec4 randomStar(vec2 c) {
    vec4 r;
	r.xy = random2(c);
	r.zw = random2(c+0.5)+0.5;
	return r;
}

/* star density */
float star_density(vec2 p) {
	p *= vec2(0.40, 0.20);
	p += simplex2d(p);
	return simplex2d(p)-0.2;
}

/* draw star in tile s fragment x */
float stars2d_tile(vec2 s, vec2 x, float scale, float theta) {
	float density = star_density(s*3.5/scale);	
	vec4 star = randomStar(s);
	
    if (star.w*1.2 > density) {
		return 0.0;
	}

	float starMagnitude = 0.7 + star.z*2.0;
	float starBrightness = 4.0 - star.z*4.0;
	vec2 v = starMagnitude*rotate2(x - star.xy, -theta);

	/* bright star with beams */
	if (scale <= 8.0) {
		v*=2.0;
		return 4.0*max(0.0, 0.5-smoothstep(0.0, 1.6, pow(dot(v,v), 0.125))) 			      
			     + max(0.0, 0.5-smoothstep(0.0, 1.0, pow(dot(v,v), 0.25)))
			     + max(0.0, 0.6-dot(abs(v), vec2(16.0, 1.0)))  // beam
			     + max(0.0, 0.6-dot(abs(v), vec2(1.0, 16.0))); // beam
	}
	
	/* cheap trick against aliasing */
	float pixels = min(1.0, 24.0/(scale*starMagnitude));
	v *= max(0.6, pixels);
	starBrightness *= pixels*pixels;
	
	float d = pow(dot(v,v), 0.25);
	return starBrightness*max(0.0, 0.5-smoothstep(0.0, 1.0, d));
}


/* 2d rectangle stars function with density */
float stars2d(vec2 p, float scale, float theta) {
	p*=scale;
	vec2 s = floor(p);
	vec2 x = p - s;
	
	return 0.0
		+stars2d_tile(s + vec2(0.0, 0.0), x - vec2(0.0, 0.0), scale, theta)
		+stars2d_tile(s + vec2(1.0, 0.0), x - vec2(1.0, 0.0), scale, theta)
		+stars2d_tile(s + vec2(0.0, 1.0), x - vec2(0.0, 1.0), scale, theta)
		+stars2d_tile(s + vec2(1.0, 1.0), x - vec2(1.0, 1.0), scale, theta)
	;
}

float sky(vec2 p, float theta) {
	p = rotate2(p, theta);
	return 0.0
		+ stars2d(p, 2.0, theta) 
		//+ stars2d(p, 4.0, theta) 
		//+ stars2d(p, 8.0, theta) 
		//+ stars2d(p, 16.0, theta)
		+ stars2d(p, 32.0, theta)
		+ stars2d(p, 64.0, theta)
		+ stars2d(p, 128.0, theta)
		+ 0.2*star_density(p*3.5)
		+ 0.15*simplex2d(p*128.0)
        ;
}

vec3 palette(float v) {
	vec3 c;

	c.b = v;
	c.g = smoothstep(0.0, 2.0, 2.0*c.b);
	c.r = smoothstep(0.0, 4.0, 4.0*c.g);
	
	return c;
}

void main(void)
{
	vec2 p = (gl_FragCoord.xy + 300.0)/(400.0); //fixed pixels
	
    float theta = iGlobalTime*0.01;
	
	gl_FragColor = vec4(palette(sky(p, theta)), 1.0);
}
