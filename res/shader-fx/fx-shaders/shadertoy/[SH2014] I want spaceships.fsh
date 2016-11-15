// https://www.shadertoy.com/view/ldjGWt

// iChannel0: t3

/*
 * Fast hack for shadertoy hackaton, copy-paste galore, don't
 * judge me over code quality, please :P
 *
 *
 */

float r;

float sdSphere( vec3 p, float s )
{
  return length(p)+s;
}

float udRoundBox( vec3 p, vec3 b, float r )
{
  return length(max(abs(p)-b,0.0))-r;
}

float sdTorus( vec3 p, vec2 t )
{
  vec2 q = vec2(length(p.xz)-t.x,p.y);
  return length(q)-t.y;
}

float sdHexPrism( vec3 p, vec2 h )
{
    vec3 q = abs(p);
    return max(q.z-h.y,max(q.x+q.y*0.57735,q.y*1.1547)-h.x);
}

vec3 rotate (vec3 vec, float angleX, float angleY, float angleZ)
{
	mat3 rotateX = mat3(1.0, 0.0, 0.0,
                        0.0, cos(angleX), -sin(angleX),
                        0.0, sin(angleX), cos(angleX));
    mat3 rotateY = mat3(cos(angleY), 0.0, sin(angleY),
                        0.0, 1.0, 0.0,
                        -sin(angleY), 0.0, cos(angleY));
    mat3 rotateZ = mat3(cos(angleZ), -sin(angleZ), 0.0,
                        sin(angleZ), cos(angleZ), 0.0,
                        0.0, 0.0, 1.0);
						
	return vec * rotateX * rotateY * rotateZ;
}

float smin( float a, float b, float k )
{
    float res = exp( -k*a ) + exp( -k*b );
    return -log( res )/k;
}

float sdTriPrism( vec3 p, vec2 h )
{
    vec3 q = abs(p);
    return max(q.z-h.y,max(q.x*0.866025+p.y*0.5,-p.y)-h.x*0.5);
}

const float PI = 3.141592;

float map( in vec3 p )
{
	vec3 rotP = rotate(p, -0.4, 0.5, 0.0);
	
	rotP.y *= 1.3;
	
	vec3 rotP2 = rotate(rotP,0.0,2.04,0.0);
	
	
	float sphere = 0.1 + sdSphere( rotP, -0.6 );
	float box1 = udRoundBox( rotP2-vec3(0.0,0.4,0.4), vec3(0.5,0.25,0.01), 0.1 );
	float box2 = udRoundBox( rotP2-vec3(0.0,0.4,0.4), vec3(0.12,0.6,0.1), 0.01 );
	float box3 = udRoundBox( rotP2-vec3(0.0,1.3,0.4), vec3(0.6,0.2,0.15), 0.1 );
	float box4 = udRoundBox( rotP2-vec3(0.0,0.0,0.4), vec3(0.6,0.4,0.2), 0.1 );
	float box5 = udRoundBox( rotP2-vec3(0.0,0.0,0.4), vec3(0.7,0.3,0.6), 0.05 );
	float box6 = udRoundBox( rotP2-vec3(0.0,0.0,-0.2), vec3(0.2,0.2,0.6), 0.03 );
	float box7 = udRoundBox( rotP2-vec3(0.0,0.0,-1.3), vec3(0.1,0.1,0.1), 0.03 );
	float prism = sdHexPrism( rotate(rotP,PI/2.0,0.0,0.0), vec2(1.0, 0.025) );
	float prism2 = sdHexPrism( rotate(rotP,PI/2.0,0.0,0.0), vec2(1.3, 0.025) );
	float prism3 = sdHexPrism( rotate(rotP,PI/2.0,0.0,0.0), vec2(1.4, 0.025) );
	float torus = sdTorus   ( rotate(rotP,0.0,0.0,0.0), vec2(1.001, 0.15) );
	float tri = sdTriPrism ( rotate(rotP,PI/2.0,0.0,0.0), vec2(2.1, 0.1 ) );
	
	
	float a = tri; // smin( torus,tri,32.0);
	//float b = smin( a, prism2, 32.0);
	//float c = smin( b, prism3, 32.0);
	float d = smin( a, box1, 32.0 );
	float e = smin( d, box2, 32.0 );
	float f = smin( e, box3, 32.0 );
	float g = smin( f, box4, 32.0 );
	float h = smin( g, box5, 32.0 );
	float i = smin( h, box6, 32.0 );
	float j = smin( i, box7, 32.0 );
	
	return j;	
}

vec3 calcNormal( in vec3 p )
{
	vec3 e = vec3(0.001,0.0,0.0);
	return normalize( vec3(map(p+e.xyy) - map(p-e.xyy),
						   map(p+e.yxy) - map(p-e.yxy),
						   map(p+e.yyx) - map(p-e.yyx) ) );
						   
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec2 p = -1.0 + 2.0*uv;
	
	p.x *= iResolution.x/iResolution.y;

    r = 2.0*texture2D( iChannel0, vec2(0.01,0.25) ).x;
	vec3 ro = vec3(0.0, 0.0, 2.4 );
	vec3 rd = normalize( vec3( p, -1.0 ) );
	
	
	vec3 col = vec3(1.0);
	
	
	float tmax = 10.0;
	float h = 1.0;
	float t = 0.0;
	for( int i=0; i<50; i++ )
	{
		if( h<0.00001 || t>tmax ) continue;
		h = map( ro + t*rd );
		t += h;
	}
	
	
	if( t<tmax )
	{
		vec3 pos = ro + t*rd;
		vec3 nor = calcNormal( pos );
		col = vec3(0.0);
		col += vec3(1.0,1.0,0.8)*max(dot( nor, vec3(0.57703) ),0.0);
		col += vec3(1.0,1.0,1.0)*0.8*max(dot( nor, vec3(0.0,-0.57703,0.6) ), 0.0);
		//col *= nor;
	}
	
	vec2 q = gl_FragCoord.xy / iResolution.xy;
	
	// vigneting
    col *= 0.5 + 0.5*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.25 );

	gl_FragColor = vec4(col,1.0);
}
