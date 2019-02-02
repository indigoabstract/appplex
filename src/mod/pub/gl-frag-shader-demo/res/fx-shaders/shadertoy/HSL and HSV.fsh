// https://www.shadertoy.com/view/lsS3Wc
// Converting from HSL and HSV color spaces to RGB. Could probably be faster, but not smaller (seems most people out there use lots of branches to do the same thing - too bad)
// Created by inigo quilez - iq/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// Somehow optimized HSV and HSL to RGB conversion functions. 

vec3 hsl2rgb( in vec3 c )
{
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );

    return c.z + c.y * (rgb-0.5)*(1.0-abs(2.0*c.z-1.0));
}

vec3 hsv2rgb( in vec3 c )
{
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
	
    return c.z * mix( vec3(1.0), rgb, c.y);
}

void main( void )
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec3 hsl = vec3( uv.x, 0.5+0.5*sin(iGlobalTime), uv.y );
	
	vec3 rgb = hsl2rgb(hsl);
	//vec3 rgb = hsv2rgb(hsl);
	
	gl_FragColor = vec4( rgb, 1.0 );
}
