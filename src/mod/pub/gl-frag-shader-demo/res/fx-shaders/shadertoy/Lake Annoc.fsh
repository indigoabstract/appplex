// https://www.shadertoy.com/view/lssSRH
vec2 op;
vec2 cart_polar(vec2);
vec2 polar_cart(vec2);
vec2 warp(vec2);
float point_distance_line(vec2, vec2, vec2);
float surface(vec2);
vec2 cart_polar(vec2 p) {
	return vec2(atan((p).y, (p).x), length(p));
}
vec2 polar_cart(vec2 p) {
	return (vec2(cos((p).x), sin((p).x))) * ((p).y);
}
vec2 warp(vec2 p) {
	return (sin((p) * 10.0)) * (((sin(iGlobalTime)) + 4.0) * 2.0);
}
float point_distance_line(vec2 a, vec2 b, vec2 point) {
	float h = clamp((dot((point) - (a), (b) - (a))) / (dot((b) - (a), (b) - (a))), 0.0, 1.0);
	return length(((point) - (a)) - (((b) - (a)) * (h)));
}
float surface(vec2 p) {
	return length((abs(polar_cart(cos(cart_polar(p))))) - (abs(op)));
}
void main() {
	op = polar_cart((cart_polar(((((gl_FragCoord).xy) / ((iResolution).xy)) * 2.0) - 1.0)) + (vec2(3.14159 / (4.0 * (cos((iGlobalTime) / 237.0))), (sin(iGlobalTime)) * 0.03)));
	float v = (surface(warp(op))) / 2.0;
	vec2 h = vec2(0.001, 0.0);
	float gv = ((abs(v)) / (length((vec2((surface(((polar_cart((cart_polar(warp(op))) + (vec2((sin(iGlobalTime)) * 3.14159, 0.0)))) * (sin((iGlobalTime) / 5.0))) + (h))) - (surface(((polar_cart((cart_polar(warp(op))) + (vec2((sin(iGlobalTime)) * 3.14159, 0.0)))) * (sin((iGlobalTime) / 5.0))) - (h))), (surface(((polar_cart((cart_polar(warp(op))) + (vec2((sin(iGlobalTime)) * 3.14159, 0.0)))) * (sin((iGlobalTime) / 5.0))) + ((h).yx))) - (surface(((polar_cart((cart_polar(warp(op))) + (vec2((sin(iGlobalTime)) * 3.14159, 0.0)))) * (sin((iGlobalTime) / 5.0))) - ((h).yx))))) / (2.0 * ((h).x))))) / 2.0;
	float macro_minmax_a = v;
	float macro_minmax_b = gv;
	float r = (min(macro_minmax_a, macro_minmax_b)) / (max(macro_minmax_a, macro_minmax_b));
	gl_FragColor = vec4(mix(gv, v, r), (mix(v, gv, r)) * 0.8, 1.0, 1.0);
}
