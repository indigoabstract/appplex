// https://www.shadertoy.com/view/XdfSz7

// iChannel1: t12

#define PI 3.14159265359
#define REC_PI .3183098862
float lenSq(vec2 c) {
	return c.x*c.x + c.y*c.y;
}
vec2 get_polar(vec2 cart) {
	vec2 pol = vec2(atan(cart.y, cart.x), log(lenSq(cart)));
	pol.x = pol.x * REC_PI * .5 + .5;
	return pol;
}
float roundTo(float x, float prec) {
	return (floor(x*prec)+.5)/prec;
}
float get_beam(vec2 pol, float prec) {
	return texture2D(iChannel1, vec2(roundTo(pol.x, prec), roundTo((pol.y+pol.x*.1)*.01-iGlobalTime*.1,prec)*.5)).r;
}
void main(void) {
	vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2. - 1.;
	vec2 pol = get_polar(uv);
	float prec = iChannelResolution[1].x;
	float beam = get_beam(pol, prec);
	beam = clamp(beam * 1024. - 920., 0., .5);
	beam *= sin((pol.x * prec - .25) * PI * 2.) * .5 + .5;
	gl_FragColor = vec4(0.0, beam * .5, beam, 1.0);
}
