// https://www.shadertoy.com/view/4dfGWM

// iChannel0: t4

const float fdelta = 2.0;
const float fhalf = 1.0;
void main(void)
{
	vec2 uv = -gl_FragCoord.xy / iResolution.xy;

	float lum = length(texture2D(iChannel0, uv).rgb);
	
	gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
	
	if (lum < 1.00) {
		if (mod(gl_FragCoord.x + gl_FragCoord.y, fdelta) == 0.0) {
			gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
	
	if (lum < 0.75) {
		if (mod(gl_FragCoord.x - gl_FragCoord.y, fdelta) == 0.0) {
			gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
	
	if (lum < 0.50) {
		if (mod(gl_FragCoord.x + gl_FragCoord.y - fhalf, fdelta) == 0.0) {
			gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
	
	if (lum < 0.25) {
		if (mod(gl_FragCoord.x - gl_FragCoord.y - fhalf, fdelta) == 0.0) {
			gl_FragColor = vec4(0.0, 0.0, 0.0, 1.0);
		}
	}
}
