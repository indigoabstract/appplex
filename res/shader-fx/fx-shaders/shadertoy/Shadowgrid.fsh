// https://www.shadertoy.com/view/Mds3D4

// iChannel0: t1

// based on noise shader by Inigo Quilez: https://www.shadertoy.com/view/XdXGW8

vec2 hash( vec2 p )
{
	p = vec2( dot(p,vec2(127.1,311.7)),
			  dot(p,vec2(269.5,183.3)) );

	float shadowShiftSpeed = 0.05;
	return -1.0 + 4.0*abs( 0.5 - fract(sin(p)*10.0+iGlobalTime*shadowShiftSpeed) );
}

float noise( in vec2 p )
{
    vec2 i = floor( p );
    vec2 f = fract( p );
	
	vec2 u = f*f*(1.0-2.0*f);

    return mix( mix( dot( hash( i + vec2(0.0,0.0) ), f - vec2(0.0,0.0) ), 
                     dot( hash( i + vec2(1.0,0.0) ), f - vec2(1.0,0.0) ), u.x),
                mix( dot( hash( i + vec2(0.0,1.0) ), f - vec2(0.0,1.0) ), 
                     dot( hash( i + vec2(1.0,1.0) ), f - vec2(1.0,1.0) ), u.x), u.y);
}

void main( void )
{
    vec2 p = gl_FragCoord.xy / iResolution.xy;
	vec2 uv = p*vec2(iResolution.x/iResolution.y,1.0);
	float f = noise( 12.0*uv );
	f = 0.5 + 0.5*f;
	
	vec3 tex = texture2D( iChannel0, uv ).xyz;
	f += tex.x*0.5;
	
	gl_FragColor = vec4( f*0.5, f*0.8, f*(0.8 + 0.2*sin(iGlobalTime*0.2)), 1.0 );
}
