// https://www.shadertoy.com/view/4slXzr
// Shows the multiples of 2, 3, 4, etc etc
// Created by inigo quilez - iq/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// workaround GLSL's mod()
float mymod( in float x, in float y )
{
	float a = mod( x, y );
	return (a==y)?0.0:a;
}
	
void main( void )
{
	vec2 p = floor( 100.0*gl_FragCoord.xy/iResolution.xx );
	
	float f = clamp( mymod(p.x, 1.0+p.y), 0.0, 1.0 );
			
	gl_FragColor = vec4( f, f, f, 1.0 );
}
