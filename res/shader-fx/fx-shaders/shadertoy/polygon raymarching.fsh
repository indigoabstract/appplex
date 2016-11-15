// https://www.shadertoy.com/view/ls2SRh
#define PI	3.14159265359
#define PI2	( PI * 2.0 )

vec2 rotate( in vec2 p, in float t )
{
	return p * cos( -t ) + vec2( p.y, -p.x ) * sin( -t );
}
    
vec3 rotate( in vec3 p, in vec3 t )
{
    p.yz = rotate( p.yz, t.x );
    p.zx = rotate( p.zx, t.y );
	p.xy = rotate( p.xy, t.z );
    return p;
}

float udBox( in vec3 p, in vec3 b)
{
	return length( max( abs( p ) - b, 0.0 ) );
}

float boundingBox( in vec3 p, in vec3 r, out bool f )
{
	const float o = 0.05;
    f = false;
	if ( abs( p.x ) > r.x) return udBox( p, r - o );
	if ( abs( p.y ) > r.y) return udBox( p, r - o );
	if ( abs( p.z ) > r.z) return udBox( p, r - o );
	f = true;
    return length( p );
}

float triangleDE( in vec3 p, in vec3 a, in vec3 b, in vec3 c  )
{
    vec3 ab = b - a;
    vec3 bc = c - b;
    vec3 ca = a - c;
    vec3 ap = p - a;
    vec3 bp = p - b;
    vec3 cp = p - c;
    vec3 n = cross( ab, -ca );
    vec3 nab = cross( n, ab );
    vec3 nbc = cross( n, bc );
    vec3 nca = cross( n, ca );
    if ( dot( nab, ap ) < 0.0 )
    {        
        float t = clamp( dot( ab, ap ) / dot( ab, ab ), 0.0, 1.0 );        
        return length( vec3( t * ab - ap ) );
    }
    if ( dot( nbc, bp ) < 0.0 )
    {
        float t = clamp( dot( bc, bp ) / dot( bc, bc ), 0.0, 1.0 );        
        return length( vec3( t * bc - bp ) );
    }
    if ( dot( nca, cp ) < 0.0 )
    {
        float t = clamp( dot( ca, cp ) / dot( ca, ca ), 0.0, 1.0 );        
        return length( vec3( t * ca - cp ) );
    }
    return abs( dot( normalize( n ), ap ) );
}

struct Mesh
{
    vec3 a;
    vec3 b;
    vec3 c;
};
    
Mesh genMesh( in int idx )
{
	float t = PI2 / 8.0;
    float i = float( idx );
    vec3 a;
    float t0, t1;
    if ( idx < 8 )
    {
        a = vec3( 0.0, 0.0, 0.3 );
        t0 = t * i;
        t1 = t * ( i + 1.0 );
    } else {
        a = vec3( 0.0, 0.0, -0.3 );
        t0 = t * ( i + 1.0 );
        t1 = t * i;
        i++;
    }   
    float r0 = mod( i, 2.0 ) * 0.5 + 0.5;
    float r1 = mod( i + 1.0, 2.0 ) * 0.5 + 0.5;
	vec3 b = vec3( r0 * cos( t0 ), r0 * sin( t0 ), 0.0 );
	vec3 c = vec3( r1 * cos( t1 ), r1 * sin( t1 ), 0.0 );   
	return Mesh( a, c, b );
}

float map(vec3 p)
{    
	bool f;
    float d = boundingBox( p, vec3( 1.0, 1.0, 0.35 ), f );
    if ( f )
    { 
        for ( int i = 0; i < 16; i++ )
    	{
			Mesh m = genMesh( i );
    		d = min( triangleDE( p, m.a, m.b, m.c ), d );
    	}
        d -= 0.001;
    }
    return d; 
}

vec3 calcNormal( in vec3 p )
{
	const vec2 e = vec2( 0.0001, 0.0 );
	vec3 n = vec3(
		map( p + e.xyy ) - map( p - e.xyy ),
		map( p + e.yxy ) - map( p - e.yxy ),
		map( p + e.yyx ) - map( p - e.yyx ) );
	return normalize( n );    
}

float march( in vec3 ro, in vec3 rd )
{
	const float maxd = 10.0;
	const float precis = 0.001;
    float h = precis * 2.0;
    float t = 0.0;
	float res = -1.0;
    for( int i = 0; i < 64; i++ )
    {
        if( h < precis || t > maxd ) break;
	    h = map( ro + rd * t );
        t += h;
    }
    if( t < maxd ) res = t;
    return res;
}

void main( void )
{
	vec2 p = ( 2.0 * gl_FragCoord.xy - iResolution.xy ) / iResolution.y;
	vec3 col = vec3( 0.4 + 0.15 * p.y );
   	vec3 rd = normalize( vec3( p, -1.8 ) );
	vec3 ro = vec3( 0.0, 0.0, 3.0 );
    vec3 light = normalize( vec3( 0.5, 0.8, 3.0 ) );
    vec3 rot = vec3( 0.2, 0.5, 0.3 ) * iGlobalTime;
    ro = rotate( ro, rot );
	rd = rotate( rd, rot );
	light = rotate( light, rot );
    float t = march( ro, rd );
    if( t > -0.1 )
    {
        vec3 pos = ro + t * rd;
        vec3 n = calcNormal( pos );
		float diff = dot( n, light );
        diff = clamp( ( diff + 0.5 ) * 0.7, 0.3, 1.0 );
		col = vec3( 1.0, 0.6, 0.6 ) *  diff;
	}
   	gl_FragColor = vec4( vec3( col ), 1.0 );
}
