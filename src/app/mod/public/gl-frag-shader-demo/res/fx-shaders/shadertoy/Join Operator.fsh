// https://www.shadertoy.com/view/MssGz2
// Created by Denis Antiga a.denis1 at yahoo.com
// Started from the sample of inigo quilez 
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// Example of an interesting Join Operator in the distance field

float sdPlaneY( vec3 p )
{
    return p.y;
}

float sdSphere( vec3 p, float s )
{
    return length(p)-s;
}




float length2( vec2 p )
{
    return sqrt( p.x*p.x + p.y*p.y );
}



//----------------------------------------------------------------------


vec2 opU( vec2 d1, vec2 d2 )
{
    return (d1.x<d2.x) ? d1 : d2;
}





vec2 opLink(float d1,float d2,float r,float c1,float c2)
{	
	float dmin;
	if (d1<d2)
		dmin=d1;
	else
		dmin=d2;
	
	float p2=(d1+d2+r*4.)*0.5;
	float r2=r*r;
	float ds1=sqrt(r2+r2)-r;
	float ds2=sqrt(r2*4.+r2)-r;
	float a=(sqrt(p2*(p2-(d1+r))*(p2-(d2+r))*(p2-2.*r)));
	
	
	if ((d1<=ds1+0.1&&d2<=ds2)||(d1<=ds2&&d2<=ds1+0.1))
		{
		dmin=a/r-r;
		return vec2(dmin,c2);
		}
	
	
	return vec2(dmin,c1);
}




//----------------------------------------------------------------------

vec2 map( in vec3 pos )
{
    vec2 res = opU( vec2( sdPlaneY(     pos), 1.0 ),
                    opLink(sdSphere(    pos-vec3( 0.0,0.25, 0.0), 0.25 ),
							sdSphere(    pos-vec3( 0.0+cos(iGlobalTime*0.3)*0.9,0.25, 0.0), 0.25 ),
							0.25,30.0,45.0) );
	
    return res;
}




vec2 castRay( in vec3 ro, in vec3 rd, in float maxd )
{
    float precis = 0.00001;
    float h=precis*2.0;
    float t = 0.0;
    float m = -1.0;
    for( int i=0; i<60; i++ )
    {
        if( abs(h)<precis||t>maxd ) continue;//break;
        t += h;
        vec2 res = map( ro+rd*t );
        h = res.x;
        m = res.y;
    }

    if( t>maxd ) m=-1.0;
    return vec2( t, m );
}


float softshadow( in vec3 ro, in vec3 rd, in float mint, in float maxt, in float k )
{
    float res = 1.0;
    float dt = 0.02;
    float t = mint;
    for( int i=0; i<30; i++ )
    {
        if( t<maxt )
        {
        float h = map( ro + rd*t ).x;
        res = min( res, k*h/t );
        t += max( 0.02, dt );
        }
    }
    return clamp( res, 0.0, 1.0 );

}

vec3 calcNormal( in vec3 pos )
{
    vec3 eps = vec3( 0.001, 0.0, 0.0 );
    vec3 nor = vec3(
        map(pos+eps.xyy).x - map(pos-eps.xyy).x,
        map(pos+eps.yxy).x - map(pos-eps.yxy).x,
        map(pos+eps.yyx).x - map(pos-eps.yyx).x );
    return normalize(nor);
}

float calcAO( in vec3 pos, in vec3 nor )
{
    float totao = 0.0;
    float sca = 1.0;
    for( int aoi=0; aoi<5; aoi++ )
    {
        float hr = 0.01 + 0.05*float(aoi);
        vec3 aopos =  nor * hr + pos;
        float dd = map( aopos ).x;
        totao += -(dd-hr)*sca;
        sca *= 0.75;
    }
    return clamp( 1.0 - 4.0*totao, 0.0, 1.0 );
}




vec3 render( in vec3 ro, in vec3 rd )
{
    vec3 col = vec3(0.0);
    vec2 res = castRay(ro,rd,20.0);
    float t = res.x;
    float m = res.y;
    if( m>-0.5 )
    {
        vec3 pos = ro + t*rd;
        vec3 nor = calcNormal( pos );

        //col = vec3(0.6) + 0.4*sin( vec3(0.05,0.08,0.10)*(m-1.0) );
        col = vec3(0.6) + 0.4*sin( vec3(0.05,0.08,0.10)*(m-1.0) );
        
        float ao = calcAO( pos, nor );

        vec3 lig = normalize( vec3(-0.6, 0.7, -0.5) );
        float amb = clamp( 0.5+0.5*nor.y, 0.0, 1.0 );
        float dif = clamp( dot( nor, lig ), 0.0, 1.0 );
        float bac = clamp( dot( nor, normalize(vec3(-lig.x,0.0,-lig.z))), 0.0, 1.0 )*clamp( 1.0-pos.y,0.0,1.0);

        float sh = 1.0;
        if( dif>0.02 ) { sh = softshadow( pos, lig, 0.02, 10.0, 7.0 ); dif *= sh; }

        vec3 brdf = vec3(0.0);
        brdf += 0.20*amb*vec3(0.10,0.11,0.13)*ao;
        brdf += 0.20*bac*vec3(0.15,0.15,0.15)*ao;
        brdf += 1.20*dif*vec3(1.00,0.90,0.70);

        float pp = clamp( dot( reflect(rd,nor), lig ), 0.0, 1.0 );
        float spe = sh*pow(pp,16.0);
        float fre = ao*pow( clamp(1.0+dot(nor,rd),0.0,1.0), 2.0 );

        col = col*brdf + vec3(1.0)*col*spe + 0.2*fre*(0.5+0.5*col);
        
    }

    col *= exp( -0.01*t*t );


    return vec3( clamp(col,0.0,1.0) );
}

void main( void )
{
    vec2 q = gl_FragCoord.xy/iResolution.xy;
    vec2 p = -1.0+2.0*q;
    p.x *= iResolution.x/iResolution.y;
    vec2 mo = iMouse.xy/iResolution.xy;
        
    float time = 15.0 + iGlobalTime;

    // camera    
    vec3 ro = vec3( -0.5+3.2*cos(0.1*time + 6.0*mo.x), 1.0 + 2.0*mo.y, 0.5 + 3.2*sin(0.1*time + 6.0*mo.x) );
    vec3 ta = vec3( -0.5, -0.4, 0.5 );
    
    // camera tx
    vec3 cw = normalize( ta-ro );
    vec3 cp = vec3( 0.0, 1.0, 0.0 );
    vec3 cu = normalize( cross(cw,cp) );
    vec3 cv = normalize( cross(cu,cw) );
    vec3 rd = normalize( p.x*cu + p.y*cv + 2.5*cw );

    
    vec3 col = render( ro, rd );

    col = sqrt( col );

    gl_FragColor=vec4( col, 1.0 );
}
