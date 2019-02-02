// https://www.shadertoy.com/view/XslSRj
// Simple black and white. Increased the amplitude of the sine wave to provide some pseudo antialiasing.
void main(void)
{
	
	float d = distance(gl_FragCoord.xy, iResolution.xy * vec2(0.5,0.5).xy);
	
	float x = sin(5.0+0.1*d + iGlobalTime*-4.0) * 5.0;
	
	// some drivers don't appear to cope with over ranged values. 
	x = clamp( x, 0.0, 1.0 );
	
	gl_FragColor = vec4( x, x, x, 1 );
}
