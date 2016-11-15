// https://www.shadertoy.com/view/XdfXRB

// iChannel0: t4

// A quick try at tinting the old film into teal and orange.
// Opposite colours on the colour-wheel.

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec3 orig = texture2D(iChannel0, uv).xyz;	// ...this is the video
	
	vec3 col = orig * orig * 1.4; 				// ...some contrast
	float bri = (col.x+col.y+col.z)/3.0;		// ...it's ok it's black and white!
	
	float v = smoothstep(.0, .7, bri);			// ...tint the dark values into cyan/teal.
	col = mix(vec3(0., 1., 1.2) * bri, col, v);
	
	v = smoothstep(.2, 1.1, bri);				// ...tint the light values into orange.
	col = mix(col, min(vec3(1.0, .55, 0.) * col * 1.2, 1.0), v);
	
	float x = (iMouse.x / iResolution.x); 
	col = mix(orig, col, step(x, uv.x));		// ...mouse X to slide filter
	gl_FragColor = vec4(min(col, 1.0),1.0);
}
