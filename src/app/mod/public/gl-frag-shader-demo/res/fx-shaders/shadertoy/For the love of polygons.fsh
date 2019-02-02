// https://www.shadertoy.com/view/4dlSRX

// iChannel0: t14
// iChannel1: t12

// mi-ku / altair

#define STEPS 50
#define EPSILON 0.001
//#define FLAT_SHADING
#define BORDER_LINES
#define BORDER_THICKNESS 1.0
//#define FLAT_GROUND
#define TRIANGLE_BG

// iq's sSqdSegment and sdTriangle functions from: https://www.shadertoy.com/view/XsXSz4
// squared distance to a segment (and orientation)
vec2 sSqdSegment( in vec2 a, in vec2 b, in vec2 p )
{
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return vec2( dot( pa-ba*h, pa-ba*h ), pa.x*ba.y-pa.y*ba.x );
}

// signed distance to a 2D triangle
float sdTriangle( in vec2 v1, in vec2 v2, in vec2 v3, in vec2 p )
{
	vec2 d = min( min( sSqdSegment( v1, v2, p ), 
					   sSqdSegment( v2, v3, p )), 
				       sSqdSegment( v3, v1, p ));

	return -sqrt(d.x)*sign(d.y);
}

// background
vec3 shadeBG( vec2 uv, float sp )
{
	float sp2 = texture2D( iChannel0, vec2( 0.25, 0.0 ) ).r;
	// TRIANGLE BG
#ifdef TRIANGLE_BG
	vec2 v2 = vec2( -0.12, 0.1 );
	vec2 v1 = vec2( 0.12, 0.1 );
	vec2 v3 = vec2( 0.0,  0.4 );
	float vadd1 = sp * 0.1;
	float vadd2 = sp2;
	float triDist1 = min( 1.0, ( 300.0 * 
						sdTriangle( v1 + vec2( vadd1, -vadd1 ), v2 + vec2( -vadd1, -vadd1 ), v3 + vec2( 0.0, vadd1 ), uv ) ) );
	if ( triDist1 < 0.0 ) // todo: optimize branch
	{
		triDist1 = pow( smoothstep( 0.0, 1.0, abs( triDist1 * 0.05 ) ), 0.1 );
	}
	triDist1 = max( 0.6, triDist1 );
	return vec3( triDist1 ) * 
		vec3( 1.0, 0.95, 0.975 );
#else
	// SPHERE BG
	uv.x *= iResolution.x / iResolution.y;
	uv.y -= 0.25;
	
	vec3 color = vec3( 1.0, 1.0, 1.0 );
	
	float l1 = pow( max( 0.0, min( 1.0, length( uv ) * 4.0 * ( 1.0 + sp  ) ) ), 10.0 );
	float l2 = pow( max( 0.0, min( 1.0, length( uv ) * 6.0 * ( 1.0 + sp2 ) ) ), 10.0 );
	return color * ( max( 1.0 - l1, l2 ) );
#endif
}
int triIsect( const vec3   V1,  // Triangle vertices
			  const vec3   V2,
			  const vec3   V3,
			  const vec3    O,  //Ray origin
			  const vec3    D,  //Ray direction
			  out float res )
{
  vec3 e1, e2;  //Edge1, Edge2
  vec3 P, Q, T;
  float det, inv_det, u, v;
  float t;
 
  e1 = V2 - V1;
  e2 = V3 - V1;

  P = cross( D, e2 );
  det = dot( e1, P );
  if(det > -EPSILON && det < EPSILON) return 0;
  inv_det = 1.0 / det;
 
  T = O - V1;
 
  u = dot(T, P) * inv_det;
  if(u < 0. || u > 1.) return 0;
 
  Q = cross( T, e1 );
 
  v = dot(D, Q) * inv_det;
  if(v < 0. || u + v  > 1.) return 0;
 
  t = dot(e2, Q) * inv_det;
 
  if(t > EPSILON) { //ray intersection
    res = t;
    return 1;
  }
 
  return 0;
}
int triIsectNC( const vec3   V1,  // Triangle vertices
			 const vec3   V2,
			 const vec3   V3,
			 const vec3    O,  //Ray origin
			 const vec3    D,  //Ray direction
			 out float res )
{
  vec3 e1, e2;  //Edge1, Edge2
  vec3 P, Q, T;
  float det, inv_det, u, v;
  float t;
 
  e1 = V2 - V1;
  e2 = V3 - V1;

  P = cross( D, e2 );
  det = dot( e1, P );
  if(det > -EPSILON && det < EPSILON) return 0;
  inv_det = 1.0 / det;
 
  T = O - V1;
 
  u = dot(T, P) * inv_det;
 
  Q = cross( T, e1 );
 
  v = dot(D, Q) * inv_det;
 
  t = dot(e2, Q) * inv_det;
 
  if(t > EPSILON) { //ray intersection
    res = t;
    return 1;
  }
 
  return 0;
}

vec3 polygonalGround( vec3 pos, float zshift, float sp )
{
	float gridSize = 1.0;
	pos.z += zshift;
	vec2 uv1 = floor( pos.xz );
	vec2 uv2 = uv1 + vec2( gridSize, gridSize );
	float um = 0.002;
	float tm = 20.0;
	float gtm = 5.0;

#ifdef FLAT_GROUND
	float h1 = 0.0;
	float h2 = 0.0;
	float h3 = 0.0;
	float h4 = 0.0;
#else
	float h1 = sin( gtm * iGlobalTime + tm * texture2D( iChannel1, um * uv1 ).r );
	float h2 = sin( gtm * iGlobalTime + tm * texture2D( iChannel1, um * vec2( uv2.x, uv1.y ) ).r );
	float h3 = sin( gtm * iGlobalTime + tm * texture2D( iChannel1, um * uv2 ).r );
	float h4 = sin( gtm * iGlobalTime + tm * texture2D( iChannel1, um * vec2( uv1.x, uv2.y ) ).r );
#endif
	
	float hm = 0.7 * max( 0.3, min( 1.0, -( uv1.y - 26.0 ) * 0.05 ) );
	vec3 v1 = vec3( uv1.x, h1 * hm, uv1.y );
	vec3 v2 = vec3( uv2.x, h2 * hm, uv1.y );
	vec3 v3 = vec3( uv2.x, h3 * hm, uv2.y );
	vec3 v4 = vec3( uv1.x, h4 * hm, uv2.y );
	float t1, t2, border1, border2;
	vec3 ro = pos + vec3( 0.0, 100.0, 0.0 );
	vec3 rd = vec3( 0.0, -1.0, 0.0 );
	int tri1res = triIsect( v1, v2, v3, ro, rd, t1 );
	int tri2res = triIsectNC( v1, v3, v4, ro, rd, t2 );
						 
	float h = 0.0;
	if ( tri1res == 1 )
	{
		vec3 pt = ro + rd * t1;
		return ( pt );
	}
	vec3 pt = ro + rd * t2;
	return pt;
}

vec3 mapGround( vec3 pos, float sp, float zshift )
{
	vec3 res = polygonalGround( pos, zshift, sp );
	float h = res.y;

	return vec3( pos.x, h * 1.5 - 4.0, pos.z );
}

float rayMarchGround( vec3 ro, vec3 rd, float sp, float zshift )
{
	float t = 0.0;
	for( int i = 0; i < STEPS; i++ )
	{
		vec3 pt = ro + rd * t;
		float h = abs( pt.y - mapGround( pt, sp, zshift ).y );
		if ( h < 0.1 )
		{
			break;
		}
		t += 0.3 * h;
	}
	return t;
}

float intersectZPlane( vec3 ro, vec3 rd, float planeZ )
{
	return ( -ro.z + planeZ ) / rd.z;
}


// ground
vec3 shadeGround( vec3 eye, vec3 pt, vec3 norm, vec3 normReflection, vec3 light, float mult, float sp )
{
#ifdef FLAT_SHADING
	pt.xz = pt.xz - mod( pt.xz, 1.0 ); // flat shading
#endif
	vec3 r = normalize( reflect( light, norm ) );
	vec3 eyeDir = normalize( pt - eye );
	float dotR = dot( r, eyeDir );
	float diffuseColor = max( 0.0, dotR );
	float ambientColor = 0.7;
	vec3 groundColor = vec3( diffuseColor + ambientColor );
	
	vec3 rd = normalize( reflect( -eyeDir, normReflection ) );
	float t = intersectZPlane( pt, rd, 10.0 );
	vec3 bgPos = pt + rd * t;
	
	float mixv = max( 0.0, dotR * 2.0 );
	vec3 bgColor = shadeBG( abs( bgPos.xy ) * vec2( 0.1, -0.2 ) + vec2( 0.0,2.0 ), sp );
	return groundColor * mixv + bgColor * ( 1.0 - mixv );
}

vec3 colorize( vec2 uv )
{
	vec3 ro = vec3( 0.0, 7.0, 0.0 );
	vec3 rd = vec3( uv.x, uv.y - 0.4, 1.0 );
	rd = normalize( rd );

	float sp = texture2D( iChannel0, vec2( 0.0, 0.0 ) ).r; // sound

	float zshift = iGlobalTime * 0.0;
	
	float t = rayMarchGround( ro, rd, sp, zshift );
	
	// directional light
	vec3 lightDir = vec3( sin( iGlobalTime ), 0.6, 0.0 ); 
	lightDir = normalize( lightDir );

	vec3 color;
	{
		vec3 pt = ro + rd * t;

		float eps = 0.001;
		vec3 norm1 = vec3( mapGround( pt + vec3( eps, 0.0, 0.0 ), sp, zshift ).y - 
						   mapGround( pt, sp, zshift ).y,
						   0.005,
						   mapGround( pt + vec3( 0.0, 0.0, eps ), sp, zshift ).y - 
						   mapGround( pt, sp, zshift ).y );
		norm1 = normalize( norm1 );
		vec3 norm2 = normalize( norm1 + vec3(0.0,8.0,0.0) );
		
		// border calculation
		float modx = abs( mod( pt.x, 1.0 ) );
		float modz = abs( mod( pt.z + zshift, 1.0 ) );
		float power = 60.0;
#ifdef BORDER_LINES
		float border = pow(     modx, power )				 // x axis border 
			         + pow( 1.0-modx, power )
			         + pow(     modz, power )				 // z axis border
			         + pow( 1.0-modz, power )				 //
					 + pow( 1.0-abs( modx - modz ), power ); // cross border
		border = max( 0.0, min( 1.0, border * BORDER_THICKNESS * sp ) );
#else
		float border = 0.0;
#endif
		
		vec3 diffuseColor = vec3( 1.0, 0.95, 0.90 );
		//vec3 diffuseColor = vec3( 1.0, 0.98, .95 );
		vec3 color1 = shadeBG( uv, sp ) * diffuseColor;
		vec3 color2 = shadeGround( ro, pt, norm1, norm2, lightDir, -1.0, sp ) * diffuseColor * ( 1.0 - border ) + vec3( .5, .5, .5 ) * border;
		float mixv = pow( min( min( 1.0, max( 0.0, -abs( pt.x ) + 7.0 ) ), min( 1.0, max( 0.0, -abs( pt.z ) + 25.0 ) ) ), 2.0 );
		color = color1 * ( 1.0 - mixv ) + color2 * mixv;
	}

	return color;
}

vec3 noiseGrain( vec2 uv )
{
	return vec3(
		texture2D( iChannel1, uv * 20.0 + vec2( iGlobalTime * 100.678, iGlobalTime * 100.317 ) ).r
	) * 0.2;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv -= vec2( 0.5, 0.5 );
	float dist = ( 1.0 - length( uv - vec2( 0.0, 0.25 ) ) * .5) * 1.0;
	vec3 color = colorize( uv ) * dist - noiseGrain( uv );

	gl_FragColor = vec4(color,1.0);
}
