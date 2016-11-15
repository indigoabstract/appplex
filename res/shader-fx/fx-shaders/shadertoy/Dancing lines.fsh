// https://www.shadertoy.com/view/Xsl3z2
#define StepSize 0.3
#define LineCount 10

//Function to draw a line, taken from the watch shader
float line(vec2 p, vec2 a, vec2 b, float thickness )
{
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);
	return 1.0 - smoothstep(thickness * 0.8, thickness * 1.2, length(pa - ba * h));
}	

void main(void)
{
	vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2.0 - 1.0;

	// convert the input coordinates by a cosinus
	// warpMultiplier is the frequency
	float warpMultiplier = (3.0 + 2.5 * sin(iGlobalTime * 0.15));
	vec2 warped = sin(uv * 6.28318530718 * warpMultiplier);

	// blend between the warpeffect and no effect
	// don't go all the way to the warp effect
	float warpornot = smoothstep(-0.5, 1.3, -sin(iGlobalTime * 0.23));
	uv = mix(uv, warped, warpornot);

	// Variate the thickness of the lines
	float thickness = pow(4.0 - 2.5 * cos(iGlobalTime * 0.067), 2.0) / iResolution.x;
	thickness *= 1.0 + (warpMultiplier * warpornot);
	float gt = floor(iGlobalTime * 15.0) * StepSize;

	// Add 10 lines to the pixel
	vec4 color = vec4(0.0, 0.0, 0.0, 1.0);
	for (int i = 0; i < LineCount; i++)
	{
		gt += StepSize;

		//Calculate the next two points
		vec2 point1 = vec2(sin(gt * 0.39), cos(gt * 0.63));
		vec2 point2 = vec2(cos(gt * 0.69), sin(gt * 0.29));

		// Fade older lines
		color.rgb = 0.95 * color.rgb;

		// Add new line
		color.rgb += line(	uv,
							point1, point2,
							thickness		)
					//With color
					* ( 0.3 +
						0.3 * vec3(	sin(gt * 3.13),
									sin(gt * 1.69),
									sin(gt * 2.67)));
}

	// Clamp oversaturation
	gl_FragColor = clamp(color, 0.0, 1.0);
}
