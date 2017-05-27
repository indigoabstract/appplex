// https://www.shadertoy.com/view/XsfXzf
// Shows dithering color when using an autoconverting sRGB rendertarget. From left to right: - signal - quantized signal, properly rounded - quantized incorrect dithered signal in linear space - quantized signal correctly dithered in srgb space.
//GAMMA_SRGB, GAMMA_2, GAMMA_24
#define GAMMA_SRGB

//note: normalized rand, [0;1[
float nrand( vec2 n )
{
  return fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}
vec3 nrand3( vec2 n ) {
	return fract( sin(dot(n.xy, vec2(12.9898, 78.233)))* vec3(43758.5453, 28001.8384, 50849.4141 ) );
}

float trunc( float a, float l )
{
	return floor(a*l)/l;
}
vec3 trunc( vec3 a, float l )
{
	return floor(a*l)/l;
}
vec4 trunc( vec4 a, float l )
{
	return floor(a*l)/l;
}
float truncRound( float a, float l )
{
	return floor(a*l+0.5)/l;
}
vec3 truncRound( vec3 a, float l )
{
	return floor(a*l+0.5)/l;
}


//CORRECT_SRGB_CONVERSION
#if defined( GAMMA_SRGB )
// see http://www.opengl.org/registry/specs/ARB/framebuffer_sRGB.txt
vec3 srgb2lin( vec3 cs )
{
	vec3 c_lo = cs / 12.92;
	vec3 c_hi = pow( (cs + 0.055) / 1.055, vec3(2.4) );
	vec3 s = step(vec3(0.04045), cs);
	return mix( c_lo, c_hi, s );
}
vec3 lin2srgb( vec3 cl )
{
	//cl = clamp( cl, 0.0, 1.0 );
	vec3 c_lo = 12.92 * cl;
	vec3 c_hi = 1.055 * pow(cl,vec3(0.41666)) - 0.055;
	vec3 s = step( vec3(0.0031308), cl);
	return mix( c_lo, c_hi, s );
}
#endif //GAMMA_SRGB

#if defined( GAMMA_2 )
vec3 lin2srgb( vec3 cl ) {
	return sqrt(cl);
}
vec3 srgb2lin( vec3 cs ) {
	return cs*cs;
}
#endif //GAMMA_2

#if defined( GAMMA_24 )
vec3 lin2srgb( vec3 cl ) {
	return pow( cl, vec3(1.0/2.4) );
}
vec3 srgb2lin( vec3 cs ) {
	return pow( cs, vec3(2.4) );
}
#endif //GAMMA_24

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	float its = uv.y * uv.y;
	
	float nrnd = nrand( uv ); //normalised noise [0;1[
	float nrnd_2 = nrand( uv + 0.597149 );
	//vec3 nrnd3 = nrand3( uv ); //rgb normalised noise [0;1[
	//float srnd =   -0.5+nrnd; //signed noise [-1;1[
	float srnd_2 = -0.5+nrnd_2;
	float trnd = nrnd + srnd_2; //"triangular" noise

	const float levels = 8.0;
	
	vec3 val = vec3(its);
	vec3 val_noise = vec3(its + nrnd/levels);
	vec3 val_noise_srgb = srgb2lin(lin2srgb(vec3(its))+nrnd/levels);

	
	//note: convert to srgb and quantize
	vec3 outval            = lin2srgb( val );
	vec3 outval_round      = truncRound( lin2srgb(val), levels );
	vec3 outval_noise      = trunc( lin2srgb(val_noise), levels );
	vec3 outval_noise_srgb = trunc( lin2srgb(val_noise_srgb), levels );
	

	//display values
	vec4 outcol = vec4(0);
	if ( uv.x < 1.0/4.0 )
		outcol.rgb = outval;
	else if ( uv.x < 2.0 / 4.0 )
		outcol.rgb = outval_round;
	else if ( uv.x < 3.0 / 4.0 )
		outcol.rgb = outval_noise;
	else if ( uv.x < 4.0 / 4.0 )
		outcol.rgb = outval_noise_srgb;

	//note: display error
	//outcol.rgb = abs( outcol.rgb - lin2srgb(val) );
		
	//display lines
	if ( abs(uv.x - 1.0/4.0) < 0.001 ) outcol = vec4(0);
	if ( abs(uv.x - 2.0/4.0) < 0.001 ) outcol = vec4(0);
	if ( abs(uv.x - 3.0/4.0) < 0.001 ) outcol = vec4(0);

	gl_FragColor = outcol;
}
