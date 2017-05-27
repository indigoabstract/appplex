// https://www.shadertoy.com/view/ldsGDM
// http://youilabs.com/blog/mobile-gpu-floating-point-accuracy-variances/
// http://blogs.arm.com/multimedia/965-benchmarking-floating-point-precision-in-mobile-gpus/

precision highp float;
void main(void) {
	float y = ( gl_FragCoord.y / iResolution.y ) * 26.0;
	float x = ( gl_FragCoord.x / iResolution.x ); 
	float b = fract( pow( 2.0, floor(y) ) + x );
	if(fract(y) >= 0.9) {
		b = 0.0;
	}
	gl_FragColor = vec4(b, b, b, 1.0 );
}
