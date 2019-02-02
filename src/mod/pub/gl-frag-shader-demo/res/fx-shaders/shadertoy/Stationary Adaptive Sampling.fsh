// https://www.shadertoy.com/view/ldXSzn
// I finally got my head around the algorithm necessary to keep samples completely stationary, interpolating new samples in. This is cool but not as simple/cheap as I would have liked, and still very much a task best done on the CPU. Move with mouse.
// A continuation of my recent shaders looking at stationary adaptive sampling, see:

// * https://www.shadertoy.com/view/XsfSRn
// * https://www.shadertoy.com/view/XdfXzn

// The white dots represent sample locations, such as distances along the pixel rays.

// Move the mouse to simulate moving the camera backwards and forwards.

// This algorithm keeps the samples completely stationary - it blends samples
// in as needed, instead of splitting / merging samples ( https://www.shadertoy.com/view/XsfSRn )

// Its a bit simpler and cheaper than the tree-based approach but its still not trivial.
// And the tree based approach works well with integrating volumes - using the distance between
// samples to integrate the volume means that sample split and merge events are trivially
// handled smoothly. On the other hand, for this approach, I think more effort will be
// required to fade samples in and out while ensuring the accumulated result stays the same.

// Since the tree based approach already works pretty well I'm parking this and moving on
// (for now at least).


// simple macros to pull mouse position in [0,1]
#define MOUSEX	iMouse.x/iResolution.x

#define CAMX (6.*(MOUSEX-.1))

#define SAMPLE_COUNT 32
#define PARAMETRIC_STEPS 32

// the gradient of the desired density line
float dens_m = .3;

float minDens = .125;

// the density line
float dens( float x )
{
	return max(minDens, dens_m * (x - CAMX) );
}
float dens_inv( float y )
{
	return y / dens_m + CAMX;
}

// the density line plot
bool fn1( float x, out float y, out vec4 col )
{
	col = vec4(1.,0.,0.,1.);
	
	y = dens( x );
	
	return true;
}

// visualise two tree levels
float inten = .4;
float visy0 = .5;
bool fn2( float x, out float y, out vec4 col )
{
		y = 0.0;
	
		col = vec4(inten);
	return true;
}

float firstX(float x)
{
	return floor(x/minDens) * minDens;
}

// this recurrence relation iteratively generates the sample locations.
// it could be optimised by storing a bumch of the values between loops
float nextX( float prevX, out float wt )
{
	// compute current sample rate
	float d = dens( prevX );
	float d0 = pow( 2., floor(log2(d)) );
	
	// move to next sample
	float nextx = prevX + d0;

	// compute next sampling rate
	d = dens( nextx );
	
	// checked if we've moved up a density level. NOTE this fails
	// for lines with gradient > 1.0, I think.
	if( d >= 2.0 * d0 )
	{
		// we've bumped up a level
		d0 *= 2.0;
		// if we are fractionally between samples, move to next whole
		nextx += mod(nextx, d0 );
	}
	
	// if we're on an odd sample, fade it out
	float sampleIdx = floor(nextx/d0);
	wt = 1. + mod( sampleIdx, 2.) * (1. - d / d0 );
	
	return nextx;
}

// these are the sample dists that will be generated. it makes much more sense
// to generate this on the CPU and pass it in.
float dists[SAMPLE_COUNT];
float weights[SAMPLE_COUNT];
void populateDists()
{
	dists[0] = firstX( CAMX + dens(CAMX) / dens_m + minDens );
	weights[0] = 1.;
	for( int i = 1; i < SAMPLE_COUNT; i++ )
	{
		dists[i] = nextX( dists[i-1], weights[i] );
	}
}

// this parametric function places a white dot at each intersection
bool pfn1( float t, out float x, out float y, out vec4 col, out float mint, out float maxt )
{
	col = vec4(1.);
	mint = 0.;
	maxt = 1.;
	
	float mindens = 0.125;
	x = dens_inv( mindens );
	float wt = 1.;
	int thisPt = int(floor(t*float(SAMPLE_COUNT)));
	for( int i = 0; i < SAMPLE_COUNT; i++ )
	{
		if( i == thisPt )
		{
			x = dists[i];
			wt = weights[i];
			break;
		}
	}
	
	vec4 col_dummy;
	fn1(x,y,col_dummy);
	
	col.a *= wt;
	//y *= wt;
	return true;
}

vec4 graph( vec2 p, float xmin, float xmax, float ymin, float ymax, float width );

void main(void)
{
	populateDists();
	
    vec2 uv = gl_FragCoord.xy / iResolution.xy;
 
	gl_FragColor = graph( uv, 0., 6./*displaySegs*y0*2.*/, 0./*y0*/, 4./*y0*2.*/, .03 );

	/* cant seem to get the visualisation of the resolutions right.. grr..
	float freq = floor( iResolution.x / (6./( minDens * pow(2.0,ceil(8.*uv.y)) ) ));
	if( fract( gl_FragCoord.x/freq ) < 2./freq )
		gl_FragColor += vec4(inten);
	*/
	return;
}


float drawNumber( float num, vec2 pos, vec2 pixel_coords );

// p is in [0,1]. 
vec4 graph( vec2 p, float xmin, float xmax, float ymin, float ymax, float width )
{
	vec4 result = vec4(0.1);
	
	float thisx = xmin + (xmax-xmin)*p.x;
	float thisy = ymin + (ymax-ymin)*p.y;
	
	// compute gradient between this pixel and next (seems reasonable)
	float eps = dFdx(thisx);

	float alpha;
	
	vec4 axisCol = vec4(vec3(.3),1.);
	
	// axes
	// x
	alpha = abs( thisy - 0. ); alpha = smoothstep( width, width/4., alpha );
	result = (1.-alpha)*result + alpha*axisCol;
	// y
	alpha = abs( thisx - 0. ); alpha = smoothstep( width, width/4., alpha );
	result = (1.-alpha)*result + alpha*axisCol;
	
	// uses iq's awesome distance to implicit http://www.iquilezles.org/www/articles/distance/distance.htm
	float f;
	vec4 fcol;
	if( fn1( thisx, f, fcol ) )
	{
		float f_1; fn1( thisx + eps, f_1, fcol ); float f_prime = (f_1 - f) / eps;
		alpha = abs(thisy - f)/sqrt(1.+f_prime*f_prime); alpha = smoothstep( width, width/4., alpha ); alpha *= fcol.a;
		result = (1.-alpha)*result + alpha*fcol;
	}
	if( fn2( thisx, f, fcol ) )
	{
		float f_1; fn2( thisx + eps, f_1, fcol ); float f_prime = (f_1 - f) / eps;
		alpha = abs(thisy - f)/sqrt(1.+f_prime*f_prime); alpha = smoothstep( width, width/4., alpha ); alpha *= fcol.a;
		result = (1.-alpha)*result + alpha*fcol;
	}
	// parametric curves. todo - join the dots!
	float x, mint, maxt;
	if( pfn1( 0., x, f, fcol, mint, maxt ) )
	{
		float dt = (maxt-mint)/float(PARAMETRIC_STEPS);
		float t = mint;
		for( int i = 0; i <= PARAMETRIC_STEPS; i++ )
		{
			pfn1( t, x, f, fcol, mint, maxt );
			alpha = length(vec2(x,f)-vec2(thisx,thisy));
			alpha = smoothstep( width, width/4., alpha ); alpha *= fcol.a;
			result = (1.-alpha)*result + alpha*fcol;
			t += dt;
		}
	}
	
	result += vec4(drawNumber(xmin, vec2(0.,0.)+vec2(1.)/iResolution.xy, p ));
	result += vec4(drawNumber(xmax, vec2(1.,0.)+vec2(-26.,1.)/iResolution.xy, p ));
	result += vec4(drawNumber(ymax, vec2(0.,1.)+vec2(1.,-7.)/iResolution.xy, p ));
	result += vec4(drawNumber(ymin, vec2(0.,0.)+vec2(1.,10.)/iResolution.xy, p ));
	
	return result;
}

// digits based on the nice ascii shader by movAX13h

float drawDig( vec2 pos, vec2 pixel_coords, float bitfield )
{
	// offset relative to 
	vec2 ic = pixel_coords - pos ;
	ic = floor(ic*iResolution.xy);
	// test if overlap letter
	if( clamp(ic.x, 0., 2.) == ic.x && clamp(ic.y, 0., 4.) == ic.y )
	{
		// compute 1d bitindex from 2d pos
		float bitIndex = ic.y*3.+ic.x;
		// isolate the bit
		return floor( mod( bitfield / exp2( floor(bitIndex) ), 2. ) );
	}
	return 0.;
}
// decimal point
float drawDecPt( vec2 center, vec2 pixel_coords )
{
	return drawDig( center, pixel_coords, 1. );
}
// minus sign
float drawMinus( vec2 center, vec2 pixel_coords )
{
	return drawDig( center, pixel_coords, 448. );
}
// digits 0 to 9
float drawDigit( float dig, vec2 pos, vec2 pixel_coords )
{
	if( dig == 1. )
		return drawDig( pos, pixel_coords, 18724. );
	if( dig == 2. )
		return drawDig( pos, pixel_coords, 31183. );
	if( dig == 3. )
		return drawDig( pos, pixel_coords, 31207. );
	if( dig == 4. )
		return drawDig( pos, pixel_coords, 23524. );
	if( dig == 5. )
		return drawDig( pos, pixel_coords, 29671. );
	if( dig == 6. )
		return drawDig( pos, pixel_coords, 29679. );
	if( dig == 7. )
		return drawDig( pos, pixel_coords, 31012. );
	if( dig == 8. )
		return drawDig( pos, pixel_coords, 31727. );
	if( dig == 9. )
		return drawDig( pos, pixel_coords, 31719. );
	// 0
	return drawDig( pos, pixel_coords, 31599. );
}

// max num width is 26px (minus, 3 nums, dec pt, 2 nums)
// max height is 6px
float drawNumber( float num, vec2 pos, vec2 pixel_coords )
{
	float result = 0.;
	bool on = false;
	float d;
	
	// minus sign
	if( num < 0. )
	{
		result += drawMinus( pos, pixel_coords );
		pos.x += 4. / iResolution.x;
		num = -num;
	}
	// hundreds
	d = floor(mod(num/100.,10.));
	if( on || d > 0. )
	{
		result += drawDigit( d, pos, pixel_coords );
		pos.x += 4. / iResolution.x;
		on = true;
	}
	// tens
	d = floor(mod(num/10.,10.));
	if( on || d > 0. )
	{
		result += drawDigit( d, pos, pixel_coords );
		pos.x += 4. / iResolution.x;
		on = true;
	}
	// ones
	d = floor(mod(num,10.));
	result += drawDigit( d, pos, pixel_coords );
	pos.x += 4. / iResolution.x;
	// dec pt
	result += drawDecPt( pos, pixel_coords );
	pos.x += 2. / iResolution.x;
	// tenths
	d = floor(mod(num/.1,10.));
	if( true )
	{
		result += drawDigit( d, pos, pixel_coords );
		pos.x += 4. / iResolution.x;
	}
	// hundredths
	d = floor(.5+mod(num/.01,10.));
	if( d > 0. )
	{
		result += drawDigit( d, pos, pixel_coords );
		pos.x += 4. / iResolution.x;
	}
	
	return clamp(result,0.,1.);
}

vec3 hsv2rgb(vec3 c);
vec3 rgb2hsv(vec3 c);

vec3 errorColour( float err, float maxerror )
{
	err = 1. - err / maxerror;
	err *= 2. / 3.;
	return hsv2rgb( vec3(err, 1., 1.) );
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
