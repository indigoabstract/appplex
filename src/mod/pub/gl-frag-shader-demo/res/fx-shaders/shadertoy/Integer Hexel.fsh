// https://www.shadertoy.com/view/ldjSDh

// iChannel0: t4

// hexel resolution
float res = 100.0 * ( sin( iGlobalTime * 0.5 ) * 0.5 + 0.5 ) + 10.0;

const vec2  D = vec2( 1.5, 1.7320508 );
const float C = 1.7320508 * 0.5;

// integer hexel coordinates
vec2 geo_to_hex( vec2 pos ) {
	vec2 hex;
    
    pos /= D;
    
    // xy
    hex.x = floor( pos.x ); float s = fract( hex.x * 0.5 ); pos.y += s;
    hex.y = floor( pos.y );
    
    // cross section
    pos -= hex + vec2( 1.0, 0.5 );
    vec2 t = step( vec2( -pos.y, pos.y ), pos.xx * D.x );
    hex += max( t.x, t.y ) * vec2( 1.0, t.x - s * 2.0 );
    
    return hex;
}

// hexel dist
float hex_dist( vec2 pos, vec2 hex ) {
    hex.y -= fract( hex.x * 0.5 );
    pos = abs( pos - hex * D - vec2( 0.5, C ) );
    return C - max( pos.y, pos.x * C + pos.y * 0.5 );
}

vec4 pixel( vec2 uv ) {
    vec2 xy = geo_to_hex( uv );
    float d = hex_dist( uv, xy );
    return texture2D( iChannel0, ( xy + 0.5 ) / ( res * vec2( 1.0, iResolution.y / iResolution.x ) ) + 0.5 )
        * mix( 0.2, 1.0, smoothstep( 0.0, 0.07, d ) )
        * min( 1.2 - smoothstep( 0.0, 1.0, length( uv / res ) ), 1.0 );
}

vec2 screen_To_geo( vec2 uv ) {
    uv  = ( uv - iResolution.xy * 0.5 ) / min( iResolution.x, iResolution.y );
    uv *= 1.0 + dot( uv, uv ) * ( cos( iGlobalTime * 0.5 ) * 0.5 + 0.5 ); // distortion
    return uv * res * D.y * 0.48;
}

void main(void)
{
    vec2 uv = gl_FragCoord.xy;
    
    gl_FragColor = 0.25 * (
        pixel( screen_To_geo( uv + vec2( -0.5, -0.5 ) ) ) +
        pixel( screen_To_geo( uv + vec2( +0.5, -0.0 ) ) ) +
        pixel( screen_To_geo( uv + vec2( -0.0, +0.5 ) ) ) +
        pixel( screen_To_geo( uv + vec2( +0.0, +0.5 ) ) )
        );
}
