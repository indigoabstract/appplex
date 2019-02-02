// https://www.shadertoy.com/view/Mdj3W3
// It appears sphere tracing can converge quickly to difficult surfaces by adding a simple correction at surface crossings (but without additional DE samples). It looks ahead one step to see if it is overstepping the surface, and if so reduces the step dist
// Overview:

// Pixel intensity shows iteration count
// Time switches between normal sphere march and the improved sphere march in this shader
// Click-drag to explore different rays
// Lower half of screen visualises the distance estimates along the ray (x-axis)
// The new code is in the raymarchFPI() function


// Full description:

// EDIT: I understand this better now. This is normal fixed point iteration but switches
// to the secant method whenever a surface crossing is detected. This has super linear
// convergence properties, and may be guaranteed to converge since I'm only applying it at
// crossings (as opposed to the normal secant method).

// For normal real-time graphics, sphere marching of distance estimated scenes
// works well enough. This is more of an academic experiment than a super-useful result.

// I found that I can get normal sphere marching to converge very well in circumstances
// that normally repel the iteration. You can see that when you aim the cross at
// a white patch and look at the iteration steps in the lower part of the screen - they
// get stuck in an orbit.

// The addition here is to check if the raymarch step will cross the surface (i.e. the
// distance estimate changes sign), and if so reduce to step size to land on the
// surface (using a linear estimation).

// The nice thing is that the raymarch loop rearranges nicely to avoid doing any
// additional DE computations (except if the step crosses the surface, in which
// case the step is reduced and the DE is evaulated at the new step location).

// A halfway approach would be to do this correction just once - when the sphere
// march crosses the surface for the first time, a correction is done using
// the estimated surface position, and then the raymarch terminates. This might be
// an easy and cheap addition to existing sphere march shaders. Future work..

// There are still situations where the iteration will completely overstep surface
// features, hence the missing parts of the bumps in the render. This can be addressed
// by reducing step size at the expense of efficiency.

// Additional Note: Since raymarch can be posed as fixed point iteration
// ( https://www.shadertoy.com/view/4ssGWl ), I think this will generalise to other 
// applications of fixed point iteration which could be really interesting. The
// correction steps seem to be equivalent to newton-raphson, using an average
// gradient (making it more robust?). All in all this approach seems to be a nice
// combination of the robustness and regularity of fixed point iteration with the
// convergence speed of newton raphson near the solution, and can be implemented
// with simple and efficient code.


#define ITERCNT 60

// turn on and off new approach over time
#define NEW_LIMIT_STEP (fract(iGlobalTime/4.)>.5)


// the wavy surface
float surfHeight( vec2 xz )
{
	float result = 3.* (cos(xz.x) + cos(xz.y));
	result *= 1.-exp(-length(xz)/10.);
	return result;
}

// evaluate the ray
vec3 rayPt( vec3 ro, vec3 rd, float t )
{
	return ro + rd * t;
}

// the distance estimate - a poor mans distance field. return the
// height difference from the surface.
float distField( vec3 pt )
{
	float dSurf = pt.y - surfHeight(pt.xz);
	
	return dSurf;
}

// raymarch
float raymarchFPI( vec3 ro, vec3 rd, out float cnt )
{
	// initial guess for t - just pick the start of the ray
	float t = 0.;
	
	float d = distField( rayPt(ro,rd,t) );
	float tNext, dNext;
	
	cnt = 0.;
	float last_t = t+10000.; // something far away from t0
	for( int i = 0; i < ITERCNT; i++ )
	{
		// termination condition - iteration has converged to surface
		if( abs(last_t - t) < 0.001 )
			continue;
		
		tNext = t + d;
		dNext = distField( rayPt(ro,rd,tNext) );
		
		// NEW ALGORITHM
		// are we crossing the surface? (sign(d) != sign(dNext)).
		// im detecting this by dividing the two and checking
		// if the result is negative. the only reason i use a divide is because
		// ill reuse the division result later. i thought it would be unstable but
		// it seems to work fine!
		float dNext_over_d = dNext/d;
		if( NEW_LIMIT_STEP && dNext_over_d < 0.0 )
		{
			// fit a line from (current t, current d) to (next t, next d),
			// and set t to the approximated intersection of the line with d=0
			
			// the human readable version
			// float grad = (dNext - d) / d;
			// d /= -grad;
			// steeper gradient means smaller step. this is analytically
			// correct (to a linear approximation of the surface)

			// optimised (confuscated) version
			d /= 1.0 - dNext_over_d;
			
			// re-evaluate at the partial step location
			tNext = t + d;
			//dNext = sign(d) * 0.00001; // use +/- epsilon as approximated dist?
			dNext = distField( rayPt(ro,rd,tNext) );
			
			// OPTION - terminate march after doing this correction step. perhaps
			// i'll visualise the error from this later
		}
		// END OF NEW ALGORITHM
		
		last_t = t;
		t = tNext;
		d = dNext;
		
		cnt += 1.;
	}
	
	return t;
}


vec3 computePixelRay( in vec2 p, out vec3 cameraPos );
vec3 hsv2rgb(vec3 c);
vec3 rgb2hsv(vec3 c);


void main(void)
{
	gl_FragColor = vec4(0.1);
	
	// get aspect corrected normalized pixel coordinate
    vec2 q = gl_FragCoord.xy / iResolution.xy;
    vec2 pp = -1.0 + 2.0*q;
	float aspect = iResolution.x / iResolution.y;
	pp.x *= aspect;
    
	if( pp.y > 0. )
	{
		// top half of screen. draw the 3d scene with a cross indicating
		// a particular ray

		pp.y = 2. * (pp.y-.5);
		
		// cross
		if( 
			(abs(pp.x) < 0.0125/aspect && abs(pp.y) < 0.1) ||
			(abs(pp.y) < 0.0125 && abs(pp.x) < 0.1/aspect) )
		{
			gl_FragColor.rgb = vec3(0.,1.,0.);
			return;
		}
		
		pp.x *= 2.;
		
		// ray march and shade based on iteration count
		vec3 ro;
		vec3 rd = computePixelRay( pp, ro );
		
		float cnt;
		float t = raymarchFPI(ro,rd, cnt);
		float iters = clamp(cnt/float(ITERCNT),0.,1.);
		
		gl_FragColor.xyz = vec3( iters );
	}
	else
	{
		// bottom half of screen. here we will draw the graph. this is done by finding
		// out which pixel we are on, computing a graph coordinate from that, and then
		// checking if we are close to the curve.
		
		// axis
		if( abs(gl_FragCoord.y - iResolution.y/4.) < 1.)
		{
			gl_FragColor.rgb = vec3(0.4);
		}
		
		// compute ray for the middle of the screen. this is where the cross
		// is located, and this is the ray that is graphed
		vec3 ro;
		vec3 rd = computePixelRay( vec2(0.), ro );
		
		// compute the t (x-axis) value for this pixel
		float tmax = 50.0;
		float thist = tmax * gl_FragCoord.x / iResolution.x;
		
		// evaluate the distance field for this value of t
		vec3 thisPt = rayPt( ro, rd, thist );
		float dist = distField( thisPt );
		
		// compute the dist (y-axis) value for this pixel
		// compute max y axis value from x axis range
		float maxDist = tmax * (iResolution.y*0.5/iResolution.x);
		float thisDist = maxDist * (pp.y+.5);
		
		// we'll also want the gradient, which tells us whether the
		// iteration will converge. compute it using forward differences
		// along the ray
		float eps = tmax/iResolution.x;
		vec3 nextPt = rayPt( ro, rd, thist + eps );
		float nextDist = distField(nextPt );
		float distGradient = (nextDist - dist) / eps;
		
		
		// when using FPI, the iterated function is t = distField + t
		// therefore the gradient of the iteration is d/dt(distField) + 1
		float fpiGrad = distGradient + 1.;
		
		// for fpi to converge, the gradient has to be in (-1,1). the next
		// few lines compute a color, blending to red over the last 20% of
		// this range
		fpiGrad = abs(fpiGrad);
		fpiGrad = smoothstep( .8, 1., fpiGrad );
		float g = 1.5 + -2.*fpiGrad;
		float r =  2.*fpiGrad;
		vec3 lineColor = clamp(vec3(r,g,0.),.0,1.);
		lineColor.g *= .85;
		
		// iq's awesome distance to implicit http://www.iquilezles.org/www/articles/distance/distance.htm
		float alpha = abs(thisDist - dist)*iResolution.y/sqrt(1.+distGradient*distGradient);
		// antialias
		alpha = smoothstep( 80., 30., alpha );
		gl_FragColor.rgb = (1.-alpha) * gl_FragColor.rgb + lineColor * alpha;
		
		
		// additional visualisation - for sphere tracing, visualise each sphere
		// need each t value, then plot circle at each t with the radius equal to the distance
		
		float stepTotalAlpha = 0.;
		
		float stept = 0.;
		
			vec3 stepPt = rayPt( ro, rd, stept );
			
			float d = distField( stepPt );
		
		float dNext;
		float tNext;
		
		float last_t = 10000.; // something far away from t0
		for( int i = 0; i < ITERCNT; i++ )
		{
			// termination condition - iteration has converged to surface
			if( abs(last_t - stept) < 0.001 )
				continue;
			
			last_t = stept;
			
			tNext = stept + d;
			dNext = distField( rayPt(ro,rd,tNext) );
			
			float dNext_over_d = dNext / d;
			if( NEW_LIMIT_STEP && dNext_over_d < 0.0 )
			{
				d /= 1.0 - dNext_over_d;
				tNext = stept + d;
				dNext = distField( rayPt(ro,rd,tNext) );
			}
			
			float stepx = -aspect + 2.*aspect * stept / tmax ;
			float stepDist = abs( d );
			float R = length( vec2(stepx,-.5) - pp );
			
			float circleR = stepDist / ( maxDist);
			// circle boundary
			float stepAlpha = 0.2*smoothstep( 5.0/iResolution.x, 0.0, abs(circleR - R) );
			// add a dot at the center
			stepAlpha += 0.3*smoothstep(5.0/iResolution.x,0.0,R);
				
			stepTotalAlpha += stepAlpha;
			
			stept = tNext;
			d = dNext;
		}
		gl_FragColor.rgb += (1.-alpha) * clamp(stepTotalAlpha,0.,1.)*vec3(1.0,1.0,0.);
	}
}






vec3 computePixelRay( in vec2 p, out vec3 cameraPos )
{
    // camera orbits around origin
	
    float camRadius = 3.8;
	// use mouse x coord
	float a = iGlobalTime*1. + .1;
	//if( iMouse.z > 0. )
	//	a = iMouse.x;
	float theta = -(a-iResolution.x)/80.;
    float xoff = camRadius * cos(theta);
    float zoff = camRadius * sin(theta);
    cameraPos = vec3(xoff,2.5,zoff);
     
    // camera target
    vec3 target = vec3(0.,4.2,0.);
     
    // camera frame
	xoff = 0.;
	float yoff = 0.;
	//if( iMouse.z > 0. )
	{
		xoff = -2.5*(iMouse.x/iResolution.x - .5);
		yoff = 4.25*(iMouse.y/iResolution.y - .5);
	}
	
	vec3 toTarget = target-cameraPos;
	vec3 right = vec3(-toTarget.z,0.,toTarget.x);
	
    vec3 fo = normalize(target-cameraPos + vec3(0.,yoff,0.) + xoff*right );
    vec3 ri = normalize(vec3(fo.z, 0., -fo.x ));
    vec3 up = normalize(cross(fo,ri));
     
    // multiplier to emulate a fov control
    float fov = .5;
	
    // ray direction
    vec3 rayDir = normalize(fo + fov*p.x*ri + fov*p.y*up);
	
	return rayDir;
}




//http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl

vec3 rgb2hsv(vec3 c)
{
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c)
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
