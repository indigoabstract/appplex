// https://www.shadertoy.com/view/MdXXzX
// Center panel is the signal. Left of center is dithered in linear space, right of center is dithered in sRGB space. Left is quantized in linear space, right is quantized in sRGB space. variation of https://www.shadertoy.com/view/XsfXzf
#define GAMMA_SRGB

//note: normalized rand, [0;1[
float nrand( vec2 n )
{
	n = floor(abs(n));
	float granularity = 1.0;
	vec2 s = floor( fract( n / (2.0 * granularity) ) * 2.0 );
	float f = (  2.0 * s.x + s.y  ) / 3.0;
	return (f - 0.5) * 0.75;
}

float round( float a, float l )
{
	return floor(a*l+0.5)/l;
}


//CORRECT_SRGB_CONVERSION
#if defined( GAMMA_SRGB )
// see http://www.opengl.org/registry/specs/ARB/framebuffer_sRGB.txt
float srgb2lin( float cs )
{
	float c_lo = cs / 12.92;
	float c_hi = pow( (cs + 0.055) / 1.055, 2.4 );
	float s = step(0.04045, cs);
	return mix( c_lo, c_hi, s );
}
float lin2srgb( float cl )
{
	float c_lo = 12.92 * cl;
	float c_hi = 1.055 * pow(cl, 0.41666) - 0.055;
	float s = step( 0.0031308, cl);
	return mix( c_lo, c_hi, s );
}
#endif //GAMMA_SRGB

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	float val = srgb2lin(uv.y);
	val = pow(val, 1.5);  // adjust the curve in linear space, so we see more dark colors
	
	
	float nrnd = nrand( gl_FragCoord.xy ); //normalised noise [0;1[


	float scale = 16.0;
	float panels = 5.0;
	
	float o;
	//display values
	vec4 outcol = vec4(0);
	if ( uv.x < 1.0/panels ) {
		// quantized signal
		o = round( val, scale );
	} else if ( uv.x < 2.0 / panels ) {
		// apply dither, then quantize signal
		float n = val + nrnd / scale;
		o = round( n, scale );
	} else if ( uv.x < 3.0 / panels ) {
		// signal
		o = val;
	} else if ( uv.x < 4.0 / panels ) {
		// non-linear transform, apply dither, then quantize signal 
		val = lin2srgb(val);
		float n = val + nrnd / scale;
		o = round( n, scale );
		o = srgb2lin(o);
	} else {
		// non-linear transform, then quantize signal 
		val = lin2srgb(val);
		o = round( val, scale );
		o = srgb2lin(o);
	}
	
	outcol.rgb = vec3(lin2srgb(o));

	//display lines
	float pad = 1.0 - floor( abs( fract( uv.x * panels ) * 2.0 - 1.0 ) + .02 ); 
	outcol *= vec4(pad);

	gl_FragColor = outcol;
}
