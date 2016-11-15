// https://www.shadertoy.com/view/ldsSRN

// iChannel1: t11

float smin( float a, float b, float k )
{
    a = pow( a, k ); b = pow( b, k );
    return pow( (a*b)/(a+b), 1.0/k );
}
float smin2( float a, float b, float k )
{
	return log( max( 0.001, exp( k*a ) ) + exp( k*b ) ) / k;
}
float cos3( vec3 a )
{
	return cos( a.x ) * cos( a.y ) * cos( a.z );
}

vec3 repeat( vec3 p, float s )
{
	return mod( p+0.5*s, s ) - 0.5*s;
}

vec3 repeatXZ( vec3 p, float s )
{
	float	h = 0.5*s;
	return vec3( mod( p.x+h, s ) - h, p.y, mod( p.z+h, s ) - h );
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel1, (uv+0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.z );
}

float fbm( vec3 p )
{
	vec3	f = vec3( 0.5, 2.0, 0.4 );
	vec3	mulF = vec3( 6.0, 8.0, 6.0 );
	const float	mulA = 0.45;
	float	a = 1.0;
	float	v  = a * noise( f*p ); f *= mulF; a *= mulA;
			v += a * noise( f*p ); f *= mulF; a *= mulA;
			v += a * noise( f*p ); f *= mulF; a *= mulA;
			v += a * noise( f*p ); f *= mulF; a *= mulA;
	return v * (1.0 / (1.0+0.5+0.25+0.125));
}

float randCell( vec2 cell, vec2 bisou )
{
	return fract( 10.0 + sin( 32151.16 * (cell.x+bisou.x) + 0.12191 )
				 + sin( -2558.5189 * (cell.y + bisou.y) + 0.59168 ) );
}

float map( vec3 p )
{
	const float	RepeatRate = 8.0;
	vec2	cellXZ = floor( (p.xz - 0.5 * RepeatRate) / RepeatRate );
	float	radius = 1.5 + 0.5 * sin( cellXZ.x + 0.564 * iGlobalTime )*sin( 1.4651 * cellXZ.y + 1.2169216* iGlobalTime );

	float	cellRandom = randCell( cellXZ, vec2( 0, 0 ) );
	vec3	height = vec3( 0.0,
						  -4.0
						  * abs( sin( 37861.32 * cellXZ.x + 2.863 * iGlobalTime )
						  *sin( 031651.0 * cellXZ.y + 1.894 * iGlobalTime ) )
						  , 0.0 );
	float	vscale = mix( 0.125, 0.8, cellRandom );
	vec3	scale = vec3( 1, vscale, 1 );
	float 	dsphere = length( repeatXZ( scale *( p + height), RepeatRate ) ) - radius;
	float 	dplane = p.y + 2.0;

	return smin2( dsphere, dplane, -1.8 );//, cos(p.x)+cos(p.y)+cos(p.z));
}

vec3 normal( vec3 p )
{
	const vec2 e = vec2( 0.001, 0.0 );
	float c = map( p );
	return normalize( vec3(
		map( p + e.xyy ) - map( p - e.xyy ),
		map( p + e.yxy ) - map( p - e.yxy ),
		map( p + e.yyx ) - map( p - e.yyx )
		) );
}

float AO( vec3 p, vec3 n )
{
	const float step = 0.01;
	p += 0.1 * n;
	float AO = 1.0;
	for ( int i=0; i < 16; i++ )
	{
		float	d = max( 0.0, map( p ) );
		p += step * n;
		
		AO *= 1.0 - exp( -20.0 * d * (2.0+float(i)) );
	}
	return AO;
}

float	Shadow( vec3 p, vec3 l )
{
	const float step = 0.5;
	p += 0.5 * l;
	float S = 1.0;
	for ( int i=0; i < 16; i++ )
	{
		float	d = max( 0.0, map( p ) );
		p += step * l;
		
		S *= 1.0 - exp( -20.0 * d * (3.0+float(i)) );
	}
	return S;
}

void main(void)
{
	vec2	uv = gl_FragCoord.xy / iResolution.xy;
	vec3	p = vec3( 5.0 * sin( 1.0 * iGlobalTime )
					 , 6.0 + 4.0 * sin( 0.5 * iGlobalTime )
					 , 4.0 * iGlobalTime );
	vec3	target = p + vec3(
		sin( 1.2 * iGlobalTime ),
		sin( 1.0 + 0.7891 * iGlobalTime ),
		5.0 + 2.0 * sin( 1.2 * iGlobalTime )
		);
	
	vec3	at = normalize( target - p );
	vec3	right = normalize( cross( at, vec3( 0, 1, 0 ) ) );
	vec3	up = cross( right, at );

	float	Tan = 0.6;
	vec3	v = normalize( vec3( iResolution.x / iResolution.y * Tan * (2.0 * uv.x - 1.0), Tan * (2.0 * uv.y - 1.0), 1.0 ) );
			v = v.x * right + v.y * up + v.z * at;

	float	t = 0.0;
	for ( int i=0; i < 64; i++ )
	{
		float	d = map( p );
		t += d;
		p += d * v;
	}
	
	if ( t <= 0.0 )
		t = 1e4;
	
	
	vec3	color = vec3( 0.0 );
	
	vec3	n = normal( p );

	vec3	Light = normalize( vec3( 1 ) );
	
	float	strip = fract( 1.0 * p.y + p.z - 1.0 * p.x );
			strip = step( strip, 0.4 ); 
	
	vec3	surfColor = mix( vec3( 1 ), vec3( 1, 0.1, 0.1 ), strip );
	
	vec3	diff = Shadow( p, Light ) * clamp( 0.0, 1.0, 0.5 + 0.5 * dot( n, Light )) * vec3( 1 );
			diff += AO( p, n ) * mix( vec3( 0.5 ), 0.2 * vec3( 0.2, 0.8, 1.0 ), (0.5 + 0.5 * n.y) );
	
	color = surfColor * diff;
	
	float fogExt = 0.025 * mix( 1.0, 1.5, 0.5 * (1.0 + noise( 0.625*vec3(1.5, 1.0, 1.5) * p + 1.0 * iGlobalTime * vec3( 0, 0, 2 ) )) );
	float fog = exp( -fogExt * t );
	color = mix( vec3( 1.0, 0.8, 0.4 ), color, fog );
	
	gl_FragColor = vec4( color, 1.0 );
}
