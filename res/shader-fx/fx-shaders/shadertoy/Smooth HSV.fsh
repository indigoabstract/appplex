// https://www.shadertoy.com/view/MsS3Wc
// C1 continuous RGB colors under linear interpolation of hue H in HSV space.
// Created by inigo quilez - iq/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.


// Converting from HSV to RGB leads to C1 discontinuities, for the RGB components
// are driven by picewise linear segments. Using a cubic smoother (smoothstep) makes 
// the color transitions in RGB C1 continuous when linearly interpolating the hue H.

// C2 continuity can be achieved as well by replacing smoothstep with a quintic
// polynomial. Of course all these cubic, quintic and trigonometric variations break 
// the standard obviously (http://en.wikipedia.org/wiki/HSL_and_HSV), but they looks 
// better.


// Official HSV to RGB conversion 
vec3 hsv2rgb( in vec3 c )
{
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );

	return c.z * mix( vec3(1.0), rgb, c.y);
}

// Smooth HSV to RGB conversion 
vec3 hsv2rgb_smooth( in vec3 c )
{
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );

	rgb = rgb*rgb*(3.0-2.0*rgb); // cubic smoothing	

	return c.z * mix( vec3(1.0), rgb, c.y);
}

// compare
void main( void )
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec3 hsl = vec3( uv.x, 1.0, uv.y );

	vec3 rgb_o = hsv2rgb( hsl );
	vec3 rgb_s = hsv2rgb_smooth( hsl );
	
	vec3 rgb = mix( rgb_o, rgb_s, smoothstep( -0.2, 0.2, sin(2.0*iGlobalTime)) );
	
	gl_FragColor = vec4( rgb, 1.0 );
}
