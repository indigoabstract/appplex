// https://www.shadertoy.com/view/ldl3R8

// iChannel0: t4

#define AMPLIFER 2.0

vec4 getPixel(in int x, in int y)
{
	return texture2D(iChannel0, (gl_FragCoord.xy + vec2(x, y)) / iResolution.xy);
}

void main(void)
{
	vec4 sum = abs(getPixel(0, 1) - getPixel(0, -1));
	sum += abs(getPixel(1, 0) - getPixel(-1, 0));
	sum /= 2.0;
	vec4 color = getPixel(0, 0);
	color.g += length(sum) * AMPLIFER;
	gl_FragColor = color;
}
