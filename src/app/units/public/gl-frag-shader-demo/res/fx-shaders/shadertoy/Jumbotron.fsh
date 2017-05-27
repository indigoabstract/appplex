// https://www.shadertoy.com/view/MdfXR8

// iChannel0: t4

float PIXEL_FACTOR = 6.0;

float smooth(float f) {
	return 32.0*f*f*(0.25*f*f-0.5*f+0.25)+0.5;	
}

void main(void)
{
	//PIXEL_FACTOR = 8.0 + 8.0 * (0.5 + 0.5 * sin(iGlobalTime * 0.25));
	vec2 chunkCoord = floor(gl_FragCoord.xy / PIXEL_FACTOR) * PIXEL_FACTOR;
	vec2 locCoord = (gl_FragCoord.xy - chunkCoord) / PIXEL_FACTOR;
	vec4 color = vec4(floor(5.0 * texture2D(iChannel0, chunkCoord / iResolution.xy).xyz) / 5.0, 1.0);
	float grey = (color.x + color.y + color.z) / 3.0;
	gl_FragColor = color * smooth(locCoord.x) * smooth(locCoord.y);
}
