// https://www.shadertoy.com/view/ldSXD1
#define PI	3.14159265359
#define PI2	( PI * 2.0 )

vec2 rotate( in vec2 p, in float t )
{
	return p * cos( -t ) + vec2( p.y, -p.x ) * sin( -t );
}   

vec3 rotateX( in vec3 p, in float t )
{
    p.yz = rotate( p.yz, t );
    return p;
}

vec3 rotateY( in vec3 p, in float t )
{
    p.zx = rotate( p.zx, t );
    return p;
}

vec3 rotateZ( in vec3 p, in float t )
{
	p.xy = rotate( p.xy, t );
    return p;
}

struct Mesh
{
    vec3 a;
    vec3 b;
    vec3 c;
};

const float sepT = 8.0;
const float sepR = 10.0;
const float radT = 0.2;
const float radR = 0.5;
const float thetaT = PI2 / sepT;
const float thetaR = PI2 / sepR;

Mesh genTorus( in int idx )
{
    float i = float( idx );
    float iT0 = mod( i, sepT );
    float iR0 = floor( i / sepT );
    float iT1 = iT0 + 1.0;
    float iR1 = iR0 + 1.0;
    float rad0 = radR + radT * cos( iT0 * thetaT );
    float rad1 = radR + radT * cos( iT1 * thetaT );
    float sin0 = sin( iR0 * thetaR );
    float sin1 = sin( iR1 * thetaR );
    float cos0 = cos( iR0 * thetaR );
    float cos1 = cos( iR1 * thetaR );    
    float h0 = radT * sin( iT0 * thetaT );
    float h1 = radT * sin( iT1 * thetaT );    
    //vec3 v0 = vec3( rad0 * sin0, h0, rad0 * cos0 );
    vec3 v1 = vec3( rad1 * sin0, h1, rad1 * cos0 );
    vec3 v2 = vec3( rad0 * sin1, h0, rad0 * cos1 );
    vec3 v3 = vec3( rad1 * sin1, h1, rad1 * cos1 );
    //if (idx < int( sepT * sepR ) ) return Mesh( v0, v1, v2 );
    return Mesh( v3, v2, v1 );
}

void main( void )
{
	vec2 p = ( 2.0 * gl_FragCoord.xy - iResolution.xy ) / iResolution.y;
    vec3 rd =normalize( vec3( p, -1.5 ) );
	vec3 ro = vec3( 0.0, -0.15, 0.8 + 0.1 * sin( iGlobalTime * 0.5 ) );
    vec3 light = normalize( vec3( 0.5, 0.8, 3.0 ) );
    float theta;
	theta = -0.7;
	ro = rotateX( ro, theta );
	rd = rotateX( rd, theta );       
    light = rotateX( light, theta );    
	theta = 0.2 * sin( iGlobalTime * 0.3 );
	ro = rotateZ( ro, theta );
	rd = rotateZ( rd, theta );       
    light = rotateZ( light, theta );    
	theta = -iGlobalTime * 0.8;
	ro = rotateY( ro, theta );
	rd = rotateY( rd, theta );       
    light = rotateY( light, theta );    
        
    vec3 col = vec3( 0.2, 0.2, 1.0 ) * ( 0.5 + 0.3 * p.y );

    vec3 far =  ro + rd * 10.0;
    vec3 nor;
    float z = 2.0;
    for (int i = 0; i <int( sepT * sepR ); i++ )
   	{
		Mesh m = genTorus( i );
        vec3 n = cross( m.c - m.a, m.b - m.a );
	    float a = dot( ro - m.a, n );
   		float b = dot( far - m.a, n );
        if ( a * b < 0.0 )
        {
    		float t = abs( a ) / ( abs( a ) + abs( b ) );
			vec3 p = ro + ( far - ro ) * t;
            if ( dot( cross( m.b - m.a, n ), p - m.a ) > 0.0 ) 
            if ( dot( cross( m.c - m.b, n ), p - m.b ) > 0.0 ) 
            if ( dot( cross( m.a - m.c, n ), p - m.c ) > 0.0 )
        	{
            	if ( z > t )
                {
                  	z = t;
                    nor = normalize( n );
                }
            }                                
        }            
    }
    if (z < 2.0)
    {     
    	col = vec3( 1.0, 0.5 + 0.2 * sin( iGlobalTime * 1.5 ), 0.2 );
        if ( dot( nor, -rd ) < 0.0 )
            col = vec3( 0.5 + 0.5 * sin( iGlobalTime ), 1.0, 0.5 + 0.5 * sin( iGlobalTime ) );
        float br = abs( dot( nor, light ) );
       	br = clamp( ( br + 0.5 ) * 0.7, 0.3, 1.0 );        
        float fog = min( 1.0, 0.01 / z / z );       
       	col *= br * fog;
    }
    gl_FragColor = vec4( col, 1.0 );
}
