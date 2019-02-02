// https://www.shadertoy.com/view/ldl3R4
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

void main(void)
{
	vec2 uv = -1.0 + 2.0*gl_FragCoord.xy / iResolution.xy;

	// bands
    float f  = step( 0.15, abs(uv.y) );
          f -= step( 0.70, abs(uv.y) );
          f *= step( abs(uv.x), 0.7 );
	

    // hearts	
	uv = fract( uv*6.0 ) - 0.5;
	uv.y -= 0.8*abs(uv.x);
	f *= step( 0.25, length( uv ) );
	
	gl_FragColor = vec4( 0.79 + 0.11*f, 0.55*f, 0.55*f, 1.0 );
}
