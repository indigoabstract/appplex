// https://www.shadertoy.com/view/XdsSDn

// iChannel0: t14

// Created by inigo quilez - iq/2013
// Heavily modified by Steven An - 2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// I've not seen anybody out there computing correct cell interior distances for Voronoi
// patterns yet. That's why they cannot shade the cell interior correctly, and why you've
// never seen cell boundaries rendered correctly. 
//
// However, here's how you do mathematically correct distances (note the equidistant and non
// degenerated grey isolines inside the cells) and hence edges (in yellow):
//
// http://www.iquilezles.org/www/articles/voronoilines/voronoilines.htm

vec3 bg = vec3(0.6, 0.8, 1.0);
vec3 white = vec3(1.0, 1.0, 1.0);
float zoom = 0.08;
vec2 fallspeed = vec2(0.04,0.05);

float pi = 3.141592653;

#define ANIMATE

float animbias = 0.5;
float animscale = 0.4;

vec2 hash( vec2 p )
{
	return texture2D( iChannel0, (p+0.5)/200.0, -100.0 ).xy;
	
	// this no longer works reliably due to a bug in some WebGL impls
//    p = vec2( dot(p,vec2(127.1,311.7)), dot(p,vec2(269.5,183.3)) );
//	return fract(sin(p)*43758.5453);
}

vec3 voronoi( in vec2 x, out vec2 cpId )
{
    vec2 n = floor(x);
    vec2 f = fract(x);

    //----------------------------------
    // first pass: regular voronoi
    //----------------------------------
	vec2 mg, mr;

    float md = 8.0;
    for( int j=-1; j<=1; j++ )
    for( int i=-1; i<=1; i++ )
    {
        vec2 g = vec2(float(i),float(j));
		vec2 o = hash( n + g );
		#ifdef ANIMATE
        o = animbias + animscale*sin( iGlobalTime*0.5 + 6.2831*o );
        #endif	
        vec2 r = g + o - f;
        float d = dot(r,r);

        if( d<md )
        {
            md = d;
            mr = r;
            mg = g;
        }
    }

    //----------------------------------
    // second pass: distance to borders
    //----------------------------------
    md = 8.0;
    for( int j=-2; j<=2; j++ )
    for( int i=-2; i<=2; i++ )
    {
        vec2 g = mg + vec2(float(i),float(j));
		vec2 o = hash( n + g );
		#ifdef ANIMATE
        o = animbias + animscale*sin( iGlobalTime*0.5 + 6.2831*o );
        #endif	
        vec2 r = g + o - f;

		
        if( dot(mr-r,mr-r)>0.000001 )
		{
        // distance to line		
        float d = dot( 0.5*(mr+r), normalize(r-mr) );

        md = min( md, d );
		}
    }
	
	cpId = n+mg;

    return vec3( md, mr );
}

float sin01(float theta)
{
	return sin(theta)*0.5 + 0.5;
}

float boxfilter( float x, float min, float max )
{
	if( x < min || x > max )
		return 0.0;
	else
		return x;
}

void main( void )
{
    vec2 p = (gl_FragCoord.xy-iResolution.xy*0.5)/iResolution.xx;
	p += iGlobalTime * fallspeed;
	
	// add variation to sky
	bg = mix( bg, vec3(0.8,0.7,0.9), pow(gl_FragCoord.y/iResolution.y, 1.0));
	
	vec2 cpid;
    vec3 c = voronoi( p/zoom, cpid );
	float centerDist = length( c.yz );
	float borderDist = c.x;

	float size = mix( 0.1, 0.20, sin01(cpid.x - cpid.y));
	
	// get angle to cp	
	float angle = atan(c.z, c.y);
	// add some animated rotation
	float angvel = sin(cpid.x*123.0+cpid.y*451.0) * 0.5*pi;
	angle += iGlobalTime * angvel;

	float numpeds = floor(mix(5.0, 9.0, sin01(cpid.x + cpid.y)));
	float pedval = sin01(angle*numpeds);
	vec3 col = mix( bg, white, pow(pedval,4.0) );
	
	// some radial details
	float numrings = floor(mix(1.0, 4.0, sin01(cpid.x*100.0 + cpid.y*42.0)));
	float ringsharp = 2.0;
	float pedval2 = pedval;
	if( hash(cpid).x < 0.5 )
		// determine inward vs. outward warped circles
		pedval2 = 1.0-pedval;
	float warpdist = mix( centerDist*0.8, centerDist, pedval2 );
	float ringval = sin01(warpdist/(size*0.8) * 2.0*pi * numrings - pi*0.5);
	col = mix( col, white, pow(ringval, ringsharp)  );
	
	// cutoff past some distance from flake center
	col = mix( col, bg, smoothstep( size*0.8, size*1.0, centerDist) );

	gl_FragColor = vec4(col,1.0);
}
