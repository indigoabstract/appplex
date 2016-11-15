// https://www.shadertoy.com/view/XdsXWN
// Based on this shader by francesco149 Use some hacks to fake double precision floats. Gets significantly better results than single precision, but I don't think it's as good as double precision.// webgl mandelbrot shader test
// by Franc[e]sco
// adapted by TekF

// my work & home PCs get very different results, so toggle some things
// it seems to be caused by the precision of the pan value
#if (1)
	// Work PC (GeForce GTX 770)
	const vec4 pan = vec4(-0.31750109, 0.48999993, 0.00000000000000588, .0);
	const float period = 175.0; // higher quality results at this position, so can zoom closer
#else
	//Home PC (Radeon HD 7700)
	const vec4 pan = vec4(-0.3175011, 0.49, .00000000011, .0);
	const float period = 142.0;
#endif

//const vec4 pan = vec4(-0.300853, 0.441, 0.00000000000000032, -0.0000000000238951); // does anyone have a good point to zoom on?
//const vec4 pan = vec4(-0.3151, 0.445, 0, -.000000013);

float aspectRatio = iResolution.x / iResolution.y;
const int maxIterations = 256;
const vec3 colourPhase = vec3(5,7,11)/80.0;
const vec3 colourPhaseStart = vec3(1);

const float initialZoom = 3.5;


vec2 DoubleMul( vec2 a, vec2 b )
{
	vec2 c;
	// c = a*b
	// (c.y+c.x) = (a.x+a.y)*(b.x+b.y);
	c.y = a.y*b.y; // smallest part
	float l = a.x*b.x; // largest part
	float r = a.x*b.y + a.y*b.x; // part in-between.
	// if we add it to the big, it might lose precision in the middle of the number
	// which would be as bad as a float, so:

// trying out some ideas to make the "doubles" more robust:
	
// try to add it to c.x, and detect how much underflowed to add to c.y
// I don't expect this will work, because the compiler will optimise it out
/*c.x = l+r;
float rf = c.x-l; // the part of r that actually made it after rounding.
r = r - rf;
c.y += r;*/
// note that a.x*b.x already underflows, so using the full precision will make that a more serious problem.
// => need upper & lower halfs of .x's... uh...

c.x = l;
c.y += r;

/*
This introduces more errors!
could try taking the difference between c.x and c.x+r, and that remainder is the value to add to c.y
// do something more robust, otherwise the vals can both lose too much precision
	float cp = log2(abs(c.x));
	float rp = log2(abs(r));
	const float precis = 20.0;
	if ( rp > cp-precis )
	{
		// chop rp up into 2 bits, put the bigger bits in the top val
		float cut = exp2(cp-precis);
		float r2 = fract(r/cut)*cut;
		c.y += r2;
		c.x += r-r2;
	}
	else
	{
		c.y += r;
	}
*/
	return c;
}

vec3 fractal( vec2 pos ) {
	
	// randomly tweaked the calculations for semi-constant zooming
	// I don't really know what I'm doing here, but it works
	float T = abs(fract((iGlobalTime/period)*.5+.5)*2.0-1.0001)*period; // using exactly 1.0 breaks it, I don't know why
	float zoom = pow(initialZoom, (-T + initialZoom + 1.0) / 5.0);

	vec4 Z = vec4(0), C = vec4(0);
	int iterations;
	bool ignore = false;
	
	// convert to texels, center the set on screen and apply zoom
	vec2 pixel;
	pixel = (pos / iResolution.xy - 0.5) * zoom; 
	pixel.y /= aspectRatio; // fix aspect ratio

	float a = iGlobalTime*.05;
	pixel = pixel*cos(a)+vec2(1,-1)*sin(a)*pixel.yx;

// to see the limit of floats, set xy instead of zw here!	
	C.zw = pixel;
	
	C -= pan;
	
	for (int i = 0; i < maxIterations; i++) {
		if (ignore)
			break;
		
		// complex number operations
		// Z = Z*Z + C
		vec4 Z2;
		//Z.x * Z.x - Z.y * Z.y, 
		Z2.xz = DoubleMul(Z.xz,Z.xz) - DoubleMul(Z.yw,Z.yw);
		Z2.yw = 2.0*DoubleMul(Z.xz,Z.yw);
		Z = Z2 + C; // apply panning
		
		// stop immediately if the point is outside a radius of 2 from (0,0) (the bounds of the mandelbrot set)
		//if ( dot((DoubleMul(Z.xz,Z.xz) + DoubleMul(Z.yw,Z.yw)),vec2(1)) > 4.0 ) // smooth
		if ( max(abs(dot(Z.xz,vec2(1))),abs(dot(Z.yw,vec2(1)))) > 2.0 ) // scallops
			ignore = true;
		
		iterations = i;
	}
	
	//return pow(sin(colourPhase.xyz * float(iterations) + colourPhaseStart)*.5+.5,vec3(2.2));
	return pow(sin(colourPhase.xyz * float(iterations) + colourPhaseStart)*.5+.5,vec3(1.5));
	//return 1.0-abs(sin(colourPhase.xyz * float(iterations) + colourPhaseStart));//*.5+.5;
}


void main()
{
	//gl_FragColor.rgb  = fractal( gl_FragCoord.xy + vec2(0,0) );

	// anti-aliasing
	gl_FragColor.rgb  = fractal( gl_FragCoord.xy + vec2(0,0) );
	gl_FragColor.rgb += fractal( gl_FragCoord.xy + vec2(.5,.0) );
	gl_FragColor.rgb += fractal( gl_FragCoord.xy + vec2(.0,.5) );
	gl_FragColor.rgb += fractal( gl_FragCoord.xy + vec2(.5,.5) );
	gl_FragColor.rgb /= 4.0;
	
/*	gl_FragColor.rgb += fractal( gl_FragCoord.xy + vec2(.25,.25) );
	gl_FragColor.rgb += fractal( gl_FragCoord.xy + vec2(.75,.25) );
	gl_FragColor.rgb += fractal( gl_FragCoord.xy + vec2(.25,.75) );
	gl_FragColor.rgb += fractal( gl_FragCoord.xy + vec2(.75,.75) );
	gl_FragColor.rgb /= 8.0;*/
	
	gl_FragColor.rgb = pow(gl_FragColor.rgb,vec3(1.0/2.2));

	gl_FragColor.a = 1.0;
}
