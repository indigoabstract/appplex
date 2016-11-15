// https://www.shadertoy.com/view/ld2Szh
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

float udBox( in vec3 p, in vec3 b )
{
	return length( max( abs( p ) - b, 0.0 ) );
}

float boundingBox( in vec3 p, in vec3 r, out bool f )
{
	const float o = 0.03;
    f = false;
    if ( abs( p.x ) > r.x ) return udBox( p, r - o );
	if ( abs( p.y ) > r.y ) return udBox( p, r - o );
	if ( abs( p.z ) > r.z ) return udBox( p, r - o );
    //if ( dot(step(r,abs(p)),vec3(1.0))>0.0 ) return udBox( p, r - o );
	f = true;
    //return length( p );
    return 1.0;
}

/*
float triangleDE( in vec3 p, in vec3 a, in vec3 b, in vec3 c )
{
    vec3 ab = b - a;
    vec3 bc = c - b;
    vec3 ca = a - c;
    vec3 ap = p - a;
    vec3 bp = p - b;
    vec3 cp = p - c;
    vec3 n = cross( ab, -ca ) ;
    if ( dot( cross( n, ab ), ap ) < 0.0 )
    {        
        float t = clamp( dot( ab, ap ) / dot( ab, ab ), 0.0, 1.0 );        
        return length( vec3( t * ab - ap ) );
    }
    if ( dot( cross( n, bc ), bp ) < 0.0 )
    {
        float t = clamp( dot( bc, bp ) / dot( bc, bc ), 0.0, 1.0 );        
        return length( vec3( t * bc - bp ) );
    }
    if ( dot( cross( n, ca ), cp ) < 0.0 )
    {
        float t = clamp( dot( ca, cp ) / dot( ca, ca ), 0.0, 1.0 );        
        return length( vec3( t * ca - cp ) );
    }
    return abs( dot( normalize( n ), ap ) );
}
*/

float dot2( in vec3 v ) { return dot(v,v); }

float udTriangle( in vec3 p, in vec3 v1, in vec3 v2, in vec3 v3 )
{
    vec3 v21 = v2 - v1; vec3 p1 = p - v1;
    vec3 v32 = v3 - v2; vec3 p2 = p - v2;
    vec3 v13 = v1 - v3; vec3 p3 = p - v3;
    vec3 nor = cross( v21, v13 );

    return sqrt( (sign(dot(cross(v21,nor),p1)) + 
                  sign(dot(cross(v32,nor),p2)) + 
                  sign(dot(cross(v13,nor),p3))<2.0) 
                  ?
                  min( min( 
                  dot2(v21*clamp(dot(v21,p1)/dot2(v21),0.0,1.0)-p1), 
                  dot2(v32*clamp(dot(v32,p2)/dot2(v32),0.0,1.0)-p2) ), 
                  dot2(v13*clamp(dot(v13,p3)/dot2(v13),0.0,1.0)-p3) )
                  :
                  dot(nor,p1)*dot(nor,p1)/dot2(nor) );
}

float udQuad( in vec3 p, in vec3 v1, in vec3 v2, in vec3 v3, in vec3 v4 )
{
    vec3 v21 = v2 - v1; vec3 p1 = p - v1;
    vec3 v32 = v3 - v2; vec3 p2 = p - v2;
    vec3 v43 = v4 - v3; vec3 p3 = p - v3;
    vec3 v14 = v1 - v4; vec3 p4 = p - v4;
    vec3 nor = cross( v21, v14 );

    return sqrt( (sign(dot(cross(v21,nor),p1)) + 
                  sign(dot(cross(v32,nor),p2)) + 
                  sign(dot(cross(v43,nor),p3)) + 
                  sign(dot(cross(v14,nor),p4))<3.0) 
                  ?
                  min( min( min( 
                  dot2(v21*clamp(dot(v21,p1)/dot2(v21),0.0,1.0)-p1), 
                  dot2(v32*clamp(dot(v32,p2)/dot2(v32),0.0,1.0)-p2) ), 
                  dot2(v43*clamp(dot(v43,p3)/dot2(v43),0.0,1.0)-p3) ),
                  dot2(v14*clamp(dot(v14,p4)/dot2(v14),0.0,1.0)-p4) )
                  :
                  dot(nor,p1)*dot(nor,p1)/dot2(nor) );
}

struct Mesh
{
    vec3 a;
    vec3 b;
    vec3 c;
    vec3 d;
};

const float sepT = 4.0;
const float sepR = 6.0;
const float radT = 0.2;
const float radR = 0.5;   
    
Mesh genTorus( in int idx )
{
    float thetaT = PI2 / sepT;
    float thetaR = PI2 / sepR;
    float i = float( idx );
    float idT = mod( i, sepT );
    vec3 vt0 = vec3( radT, 0.0, 0.0 );
    vec3 vt1 = vec3( radT, 0.0, 0.0 );
    vt0.xy = rotate( vt0.xy, idT * thetaT );
    vt1.xy = rotate( vt1.xy, ( idT + 1.0 ) * thetaT );
    vt0.x += radR;
    vt1.x += radR;
    float idR = floor( i / sepT );
    vec3 v0 = vt0;
    vec3 v1 = vt1;
    vec3 v2 = vt0;
    vec3 v3 = vt1;
    v0.zx = rotate( v0.zx, idR * thetaR );
    v1.zx = rotate( v1.zx, idR * thetaR );
    v2.zx = rotate( v2.zx, ( idR + 1.0 ) * thetaR );
    v3.zx = rotate( v3.zx, ( idR + 1.0 ) * thetaR );
    //if (idx < int( sepT * sepR ) ) return Mesh( v0, v1, v2 );
    //return Mesh( v2, v3, v1 );
    return Mesh( v1, v0, v2, v3 );
}

/*

vec3 calcNormal( in vec3 p )
{
	const vec2 e = vec2( 0.0001, 0.0 );
	vec3 n = vec3(
		map( p + e.xyy ) - map( p - e.xyy ),
		map( p + e.yxy ) - map( p - e.yxy ),
		map( p + e.yyx ) - map( p - e.yyx ) );
	return normalize( n );    
}

float map( in vec3 p )
{
    bool f;
    float d = boundingBox( p, vec3( 0.72, 0.22, 0.72 ), f );
    if ( f )
    { 
        for ( int i = 0; i < 30; i++ )
    	{
            Mesh m = genTorus( i );
            d = min( udQuad( p, m.a, m.b, m.c, m.d ), d );
        }
        d -= 0.001;
    }
    vec3 a = vec3(0.0, 1.0, 0.0);
    vec3 b = vec3(1.5, 0.0, 0.0);
    vec3 c = vec3(0.0, -1.0, 0.0);
    return min( d, udTriangle( p, a, b, c ) - 0.001 ); 
}
*/

vec3 calcNormal( in vec3 p, in vec3 v1, in vec3 v2 )
{
    return normalize( cross(  v2 - v1, p - v1 ) );
}
    
float map( in vec3 p, out vec3 nor )
{
    bool f;
    vec3 v1 = vec3( 0.0 );
    vec3 v2 = vec3( 0.0 );
    nor = vec3( 0.0 );
    float de = boundingBox( p, vec3( 0.72, 0.22, 0.72 ), f );
    if ( f )
    { 
        for ( int i = 0; i < int(sepT*sepR); i++ )
    	{
            Mesh m = genTorus( i );
            float d0 = udQuad( p, m.a, m.b, m.c, m.d );
            if ( d0 < de )
            {
              de = d0;
              v1 = m.a;
              v2 = m.b;
            }
        }
        de -= 0.001;
    }
    vec3 t1 = vec3(1.5, 0.0, 0.0);
    vec3 t2 = vec3(0.0, 1.0, 0.0);
    vec3 t3 = vec3(0.0, -1.0, 0.0);
    float d0 = udTriangle( p, t1, t2, t3 ) - 0.001;
    if ( d0 < de )
    {
    	de = d0;
        v1 = t1;
        v2 = t2;       
    }
    vec3 q1 = vec3(0.0, 0.0, 0.0);
    vec3 q2 = vec3(-1.0, 0.8, 0.0);
    //vec3 q3 = vec3(-1.2 + 0.5 * sin(iGlobalTime), 0.0, 0.0); // concave quad
    vec3 q3 = vec3(-1.2, 0.0, 0.0);
    vec3 q4 = vec3(-1.0, -0.8, 0.0);
    float d1 = udQuad( p, q1, q2, q3, q4 ) - 0.001;
    if ( d1 < de )
    {
        de = d1;
        v1 = q1;
        v2 = q2;       
    }
    nor = calcNormal( p, v1, v2 );
    return de; 
}


float march( in vec3 ro, in vec3 rd, out vec3 nor )
{
	const float maxd = 5.0;
	const float precis = 0.001;
    float h = precis * 2.0;
    float t = 0.0;
	float res = -1.0;
    for( int i = 0; i < 48; i++ )
    {
        if( h < precis || t > maxd ) break;
	    //h = map( ro + rd * t );
	    h = map( ro + rd * t, nor );
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
    vec3 rot = vec3( 0.3, 0.5 * iGlobalTime, 0.4 );
    ro = rotate( ro, rot );
	rd = rotate( rd, rot );
	light = rotate( light, rot );
    vec3 n;
    //float t = march( ro, rd );
    float t = march( ro, rd, n );
    if( t > -0.1 )
    {
        //vec3 pos = ro + t * rd;
        //vec3 n = calcNormal( pos );
        float br = dot( n, light ) ;
		col = vec3( 1.0, 0.8, 0.5 );
        //float side = dot( n, rotate( vec3(0.0, 0.0, 1.0 ), rot));
        float side = dot( n, -rd);
        if (side < 0.0) // reverse
        {
            col = vec3( 0.5, 0.5, 1.0 );
            br *= -1.0;
            // br = abs( br );
        }
        br = clamp( ( br + 0.5 ) * 0.7, 0.3, 1.0 );
        col*=  br; 
	}
   	gl_FragColor = vec4( vec3( col ), 1.0 );
}
