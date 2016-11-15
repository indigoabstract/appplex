// https://www.shadertoy.com/view/XssXR4
// float to RGBA8 is used in threejs to store depth in an RGBA texture. Artefacts are due to the error introduced during the packing : floor and fract seem to work incorrectly with big values. See packR for different values of offset.
float packA( float depth ) {

	return floor(256.0 * depth) / 256.0;
}

float packB( float depth ) {

	return floor(256.0 * fract(256.0 * depth)) / 256.0;
}

float packG( float depth ) {

	return floor(256.0 * fract(256.0 * 256.0 * depth)) / 256.0;
}

float packR( float depth ) {

	float tmp = 256.0 * 256.0 * 256.0 * depth;
	return tmp - floor(tmp);
	//return floor(256.0 * tmp) / 256.0;
}

vec4 packFloatOld( float depth ) {

  const vec4 bit_shift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0 );
  const vec4 bit_mask  = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0 );

  vec4 res = fract( depth * bit_shift );

  res -= res.xxyz * bit_mask;
  return res;
}

vec4 packFloat( float depth ) {

  const vec4 bit_shift = vec4(256.0 * 256.0 * 256.0, 256.0 * 256.0, 256.0, 1.0 );
  const vec4 bit_mask  = vec4(0.0, 1.0 / 256.0, 1.0 / 256.0, 1.0 / 256.0 );

  vec4 res = mod( depth * bit_shift * vec4( 255 ), vec4( 255 ) ) / vec4( 255 );

  res -= res.xxyz * bit_mask;
  return res;
}

float unpackFloat( vec4 depth ) {

	vec4 bitSh = vec4(1.0 / (256.0 * 256.0 * 256.0), 1.0 / (256.0 * 256.0), 1.0 / 256.0, 1.0);
	return dot(depth, bitSh);
}

void main(void)
{
	float scale = 1.0 / pow(2.0, iGlobalTime);
	float offset = 0.5 - 0.5 * scale;
    vec2 p = offset + scale * gl_FragCoord.xy / iResolution.xy;

	vec4 packedFloatOld = packFloatOld(p.x);
	vec4 packedFloat = packFloat(p.x);
	//float packedR = packR(p.x);
	//float packedG = packG(p.x);
	//float packedB = packB(p.x);
	//float packedA = packA(p.x);
	//float error = 1000000.0*abs(unpackFloat(vec4(0.0, packedG, packedB, packedA)) - p.x);
	//float error = clamp(100000.0*abs(unpackFloat(packedFloat) - p.x),0.0,1.0);
    vec4 error = abs(packedFloatOld - packedFloat);
	gl_FragColor = vec4(vec3(error.x), 1.0);
}
