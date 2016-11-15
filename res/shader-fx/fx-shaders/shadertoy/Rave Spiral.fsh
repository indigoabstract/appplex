// https://www.shadertoy.com/view/4ssXW7
// Having fun with distance fields over polar coords.
vec2 cart_logpolar(vec2 p) {
	return vec2(atan((p).y, (p).x), log(length(p)));
}
vec3 hsv_rgb(vec3 hsv) {
	return (mix(vec3(1.0, 1.0, 1.0), clamp((abs((mod((((hsv).x) / 60.0) + (vec3(0.0, 4.0, 2.0)), 6.0)) - 3.0)) - 1.0, 0.0, 1.0), (hsv).y)) * ((hsv).z);
}
vec2 polar_norm(vec2 p) {
	return vec2(mod(((p).x) + 6.28318, 6.28318), (p).y);
}
vec2 logpolar_cart(vec2 p) {
	return (vec2(cos((p).x), sin((p).x))) * (pow(2.71828, (p).y));
}
vec4 distance_field(vec2 p) {
	float a = ((p).y) * ((sin(((iGlobalTime) / 10.0) + (((p).y) + (iGlobalTime)))) * 2.0);
	vec2 xp = logpolar_cart(((p) + (vec2(a, 0.0))) - ((mod(polar_norm(((p) - (iGlobalTime)) + (vec2(a, 0.0))), 0.314159)) - 0.1570795));
	vec2 t = (abs((mod(polar_norm(((p) - (iGlobalTime)) + (vec2(a, 0.0))), 0.314159)) - 0.1570795)) - 0.1570795;
	return vec4(mix((min(max((t).x, (t).y), 0.0)) + (length(max(t, 0.0))), (length((mod(polar_norm(((p) - (iGlobalTime)) + (vec2(a, 0.0))), 0.314159)) - 0.1570795)) - 0.1570795, abs(sin(((xp).y) * 10.0))), hsv_rgb(vec3(abs((sin((((xp).y) + ((xp).x)) * 10.0)) * 360.0), abs(sin(((xp).x) * 17.0)), abs(sin(((xp).y) * 13.0)))));
}
void main() {
	vec2 p = cart_logpolar((((((gl_FragCoord).xy) / ((iResolution).xy)) * 2.0) - 1.0) * (vec2(1.0, ((iResolution).y) / ((iResolution).x))));
	vec2 h = vec2(0.001, 0.0);
	vec3 mat = (distance_field(p)).yzw;
	gl_FragColor = vec4(vec3((clamp((- (((distance_field(p)).x) / (abs(length((vec2(((distance_field((p) + (h))).x) - ((distance_field((p) - (h))).x), ((distance_field((p) + ((h).yx))).x) - ((distance_field((p) - ((h).yx))).x))) / (2.0 * ((h).x))))))) * 20.0, 0.0, 1.0)) * (mat)), 1.0);
}
