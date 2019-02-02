// https://www.shadertoy.com/view/MdXSWX
#define PI	3.14159265359
#define PI2	( PI * 2.0 )

vec2 rotate( vec2 p, float t )
{
	return p * cos( -t ) + vec2( p.y, -p.x ) * sin( -t );
}
    
vec3 rotate( vec3 p, vec3 t )
{
    p.yz = rotate( p.yz, t.x );
    p.zx = rotate( p.zx, t.y );
	p.xy = rotate( p.xy, t.z );
    return p;
}

vec3 perspective( vec3 p )
{
	float l = 3.0;
	float t = l / ( l - p.z );
    p.xy *= t;
    p.z *= -1.0;
	return p ;	
}

struct Mesh
{
    vec3 a;
    vec3 b;
    vec3 c;
    vec3 n;
};
    
float cross( vec2 a, vec2 b ) 
{
    return a.x * b.y - b.x * a.y;
}
    
int exist( Mesh m, vec2 p )
{    
	float xa = cross( m.b.xy - m.a.xy, p - m.a.xy );
	float xb = cross( m.c.xy - m.b.xy, p - m.b.xy );
	float xc = cross( m.a.xy - m.c.xy, p - m.c.xy );
	if ( xa > 0.0 && xb > 0.0 && xc > 0.0 ) return 1; // front face
	if ( xa < 0.0 && xb < 0.0 && xc < 0.0 ) return 0; // back face
	return -1;
}

vec3 getNormal( vec3 a, vec3 b, vec3 c )
{
	return normalize( cross( c - a, b - a ) );
}

float getZ( vec3 a, vec3 b, vec3 c, vec2 p )
{
    vec3 n = getNormal( a, b, c );
    return ( dot( n, a ) - dot( n.xy, p ) ) / n.z;
}

Mesh genMesh( int idx )
{
	float t = PI2 / 10.0;
    float i = float( idx );
    vec3 a = vec3(0.);
    float t0=0., t1=0.;
    if ( idx < 10 )
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
    vec3 rot = vec3( 0.0, 0.5, 0.3 ) * iGlobalTime;
    a = rotate( a, rot );
    b = rotate( b, rot );
    c = rotate( c, rot );
    vec3 n =  getNormal(a, b, c);
    vec3 roc = vec3( 0.0, 0.0, -2.0 );  
    a += roc;
    b += roc;
    c += roc;
    a = perspective( a );
    b = perspective( b );
    c = perspective( c );
	return Mesh( a, c, b, n );
}

void main( void )
{
	vec2 p = ( 2.0 * gl_FragCoord.xy - iResolution.xy ) / iResolution.y;
	vec3 col = vec3( 0.4 + 0.15 * p.y );
    for ( int i = 0; i < 20; i++ )
    {
        Mesh m = genMesh( i );
        float s = 1.1;
        m.a.xy *= s;
        m.b.xy *= s;
        m.c.xy *= s;
        if ( exist( m , p ) == 0 ) col = vec3( 0.0 );
    }
    float fz = -100.0;
    vec3 light = normalize( vec3( 0.5, 0.5, 1.0 ) );      
    for ( int i = 0; i < 20; i++ )
    {
        Mesh m = genMesh(i);
        if ( exist( m, p ) == 1 )
        //if ( exist( m, p ) > -1 )
        {
            float z = getZ( m.a, m.b, m.c, p );
            if ( z > fz )
            {
                fz = z;
	    		float t = dot( m.n, light );
            	col = vec3( 1.0, 0.8, 0.2 ) * clamp( ( t + 0.5 ) * 0.7, 0.3, 1.0 );
            }
    	}
    }
    gl_FragColor = vec4( col, 1.0 );
}
