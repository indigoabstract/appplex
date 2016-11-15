// https://www.shadertoy.com/view/MsXXDf

// iChannel0: t12
// iChannel1: t14

const int max_iterations = 100;
const float stop_threshold = 0.001;
const float grad_step = 0.01;
const float clip_far = 100.0;

const vec3 worldUp = vec3( 0, 1, 0 );
const vec3 worldRight = vec3( 1, 0, 0 );
const vec3 worldForward = vec3( 0, 0, 1 );

const float PI = 3.14159265359;
const float DEGS2RADS = PI / 180.0;

struct HitInfo
{
    float distance;
    int id;
};

    
float hash( vec2 p )
{
    float h = dot(p,vec2(127.1,311.7));
    
    return -1.0 + 2.0*fract(sin(h)*43758.5453123);
}

float noise( in vec2 p )
{
    vec2 i = floor( p );
    vec2 f = fract( p );
    
    vec2 u = f*f*(3.0-2.0*f);

    return mix( mix( hash( i + vec2(0.0,0.0) ), 
                     hash( i + vec2(1.0,0.0) ), u.x),
                mix( hash( i + vec2(0.0,1.0) ), 
                     hash( i + vec2(1.0,1.0) ), u.x), u.y);
}

float texNoise0( in vec3 x, float lod_bias )
{   
    vec3 p = floor( x );
    vec3 f = fract( x );
    f = f * f * ( 3.0 - 2.0 * f );
    vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
    vec2 rg = texture2D( iChannel0, uv*(1./256.0), lod_bias ).yx;

    return mix( rg.x, rg.y, f.z );
}

float texNoise1( in vec3 x, float lod_bias )
{   
    vec3 p = floor( x );
    vec3 f = fract( x );
    f = f * f * ( 3.0 - 2.0 * f );
    vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
    vec2 rg = texture2D( iChannel1, uv*(1./256.0), lod_bias ).yx;

    return mix( rg.x, rg.y, f.z );
}

vec4 BlendUnder(vec4 accum,vec4 col)
{
    col = clamp( col, vec4( 0 ), vec4( 1 ) );   
    accum += vec4( col.rgb * col.a, col.a ) * ( 1.0 - accum.a );   
    return accum;
}

float sdSphere( vec4 pos, vec4 origin, float r ) 
{
    return length( pos - origin ) - r;
}

float udBox( vec4 p, vec4 center, vec3 dimensions )
{
    p.x = mod( p.x, 4.0 ) - 0.5;
    return length( max( abs( p - center ).xyz - dimensions, 0.0 ) );
}

float sdPlane( vec3 p, vec4 n )
{
  // n must be normalized
  return dot( p, n.xyz ) + n.w;
}

// get distance in the world
HitInfo dist_field( vec3 pos ) 
{
    float d0 = sdSphere( vec4( pos, 1 ), vec4( -0.5, 0.0, 3.5, 1 ), 1.0 );
    float d1 = sdSphere( vec4( pos, 1 ), vec4( 4.5, 0.0, 0.5, 1 ), 1.0 );
    float dBox = udBox( vec4( pos, 1 ), vec4( 1.5, 0, 1.5, 1 ), vec3( 0.2, 100, 0.2 ) );
    float dPlane = sdPlane( pos, vec4( 0, 1, 0, 1 ) );
    
    float df1 = min( d1, min( dBox, d0 ) );
    if( dPlane < df1 )
        return HitInfo( dPlane, 0 );
    else 
        return HitInfo( df1, 1 );
}

// get gradient in the world
vec3 gradient( vec3 pos ) 
{
    const vec3 dx = vec3( grad_step, 0.0, 0.0 );
    const vec3 dy = vec3( 0.0, grad_step, 0.0 );
    const vec3 dz = vec3( 0.0, 0.0, grad_step );
    return normalize(   vec3(   dist_field( pos + dx ).distance - dist_field( pos - dx ).distance,
                                dist_field( pos + dy ).distance - dist_field( pos - dy ).distance,
                                dist_field( pos + dz ).distance - dist_field( pos - dz ).distance ) );
}

HitInfo rayMarch( vec3 origin, vec3 dir, float farClip )
{
    float rayDistance = 0.0;
    for( int i = 0; i < max_iterations; i++ ) 
    {
        HitInfo hit = dist_field( origin + dir * rayDistance );
        float d = hit.distance;
        if ( d <= stop_threshold )
            return HitInfo( rayDistance, hit.id );
  
        rayDistance += d;
    }
    
    return HitInfo( rayDistance, 0 ); // hack: default material is zero?
}

float softShadowRayMarch( vec3 origin, vec3 dir ) 
{
    float rayDistance = 0.0;
    float penumbra = 1.0;
    for( int i = 0; i < max_iterations; i++ ) 
    {
        float d = dist_field( origin + dir * rayDistance ).distance;
        if ( d <= stop_threshold )
            return 0.0;
        penumbra = min( penumbra, 15.0 * d / rayDistance );
        rayDistance += d;
    }
    
    return penumbra;
}

vec4 rotYZ( vec4 v, float thetaDegs )
{
    mat4 m = mat4( 1 );
    float thetaRads = DEGS2RADS * thetaDegs;
    m[ 1 ].yz = vec2( cos( thetaRads ), -sin( thetaRads ) );
    m[ 2 ].yz = vec2( sin( thetaRads ), cos( thetaRads ) );

    return m * v;
}

vec3 calcLighting( vec3 n, vec3 v, vec3 l )
{
    vec3 Cdiff = vec3( 0.9, 0.8, 0.8 );
    vec3 Cspec = vec3( 0.9, 0.6, 0.5 );
    vec3 h = ( v + n ) / 2.0;
    float m = 1.0;
    vec3 light = ( Cdiff + ( m + 8.0 ) * 0.125 * pow( max( dot( n, h ), 0.0 ), m ) ) * 0.6183 * max( dot( n, l ), 0.0 );
    
    // lighting and lightning ===================
    if( mod( iGlobalTime, 5.0 ) > 4.0 )
        return light * 1.65;
    else
        return light;
}

float groundTex( vec3 point, vec2 scale )
{
    vec2 uv = point.xz * scale;
    mat2 m = mat2( 1.6,  1.2, -1.2,  1.6 );
    float f  = 0.5000*noise( uv ); uv = m*uv;
    f += 0.2500*noise( uv ); uv = m*uv;
    f += 0.1250*noise( uv ); uv = m*uv;
    f += 0.0625*noise( uv ); uv = m*uv;
    f = 0.5 + 0.5*f;

    f *= smoothstep( 0.0, 0.005, abs(point.x-0.6) );
    return f;
}

void main(void)
{
    float ar = iResolution.x / iResolution.y;
    float theta = -31.0;
    
    vec2 ndcPoint = vec2( ( gl_FragCoord.x / iResolution.x - 0.5) * ar, gl_FragCoord.y / iResolution.y - 0.5 );
    vec3 dir = normalize( vec3( ndcPoint, 1) );

    dir = normalize( rotYZ( vec4( dir, 0 ), theta ).xyz );
    vec3 eye = vec3( 1.5, -10, 1.5 ) - 15.0 * normalize( rotYZ( vec4( worldForward, 1 ), -theta ) ).xyz;
    eye.y = 2.0;

    HitInfo hit = rayMarch( eye, dir, clip_far );
    if( hit.distance >= clip_far )
        discard;
    
    float s = 1.0 - hit.distance / clip_far;
    vec3 point = eye + dir * hit.distance;
    vec3 lightDir = normalize( vec3( 1, 1, 1 ) );
    vec3 n = gradient( point );

    s *= softShadowRayMarch( point + lightDir * 0.01, lightDir );
    
    // falling rain ===============================================
    float last_t = 0.;
    vec4 accum = vec4( 0, 0, 0, 0);
    for( int i = 0; i < 100; i++ )
    {                           
        float t = float( i ) * 0.04;

        vec3 p = eye + t * dir;
        vec3 uvw = p;
        uvw.y /= 10.0;
        uvw.y += iGlobalTime;
        uvw *= 30.0;
        
        float dens = pow( texNoise0(uvw,-1000.), 6. + abs( 2. * sin( noise( vec2( iGlobalTime, iGlobalTime ) ) ) )  );
        dens -= 0.25;
        dens *= ( t-last_t )* 1.5;
        
        accum = BlendUnder( accum, vec4( 1, 1, 1, dens ) );
            
        last_t = t;
    }

    // rain hitting the ground =============================================
    vec3 uvw = point;
    uvw *= 20.0;
    uvw.y += iGlobalTime * 20.0;
    float dens = texNoise1( uvw, -100.0 );
    dens = pow( dens, 5. );
    dens=sin(dens);
    dens *= 0.4;
    accum = BlendUnder( accum, vec4( 1, 1, 1, dens ) );

    vec3 col = vec3( 0 );

    // ground plane =====================================
    if( hit.id == 0)
    {
        vec3 light = vec3( s, s, s + 0.1);

        // reflections =====================
        vec3 r = reflect( dir, normalize( worldUp.xyz + 0.2 * vec3( dens, 0, dens ) ) );
        hit = rayMarch( point + r * 0.01, r, clip_far );
        if( hit.distance < clip_far )
            light *= calcLighting( n, dir, lightDir );
        
        float f = groundTex( point, vec2( 0.125, 0.125 ) );
        
        light *= f;
        col = light + vec3( mod( iGlobalTime, 5.0 ) > 4.0 ? 0.05 : 0. );
        col = BlendUnder( accum, vec4( col, 1. ) ).rgb;
    }
    else // other objects
    {
        // lighting ===================================================
        col = calcLighting( n, dir, lightDir );

        col.rgb *= s;
        col += vec3( mod( iGlobalTime, 5.0 ) > 4.0 ? 0.05 : 0. );
        col = BlendUnder( accum, vec4( col, 1. ) ).rgb;
    }

    gl_FragColor = vec4( col, 1.0 );
}
