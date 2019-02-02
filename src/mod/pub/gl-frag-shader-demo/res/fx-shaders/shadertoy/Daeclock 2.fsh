// https://www.shadertoy.com/view/XssXDM
vec2 cart_polar(vec2 p) {
	return vec2(atan((p).y, (p).x), length(p));
}
vec2 polar_norm(vec2 p) {
	return vec2(mod(((p).x) + 6.28318, 6.28318), (p).y);
}
float arc(vec2 p, vec3 d) {
	vec2 t = (abs(polar_norm((cart_polar((p) * (vec2(-1.0, 1.0)))) - (vec2(1.570795, (d).z))))) - (vec2(((d).x) + ((clamp(((d).x) - 6.28317, 0.0, 1.0)) * 10000000.0), (d).y));
	return (min(max((t).x, (t).y), 0.0)) + (length(max(t, 0.0)));
}
vec4 distance_field(vec2 p) {
	float secondpos = (floor(mod((iDate).w, 60.0))) / 60.0;
	float minutepos = ((mod(floor(((iDate).w) / 60.0), 60.0)) / 60.0) + ((secondpos) / 60.0);
	float hourpos = ((mod(floor((((iDate).w) / 60.0) / 60.0), 12.0)) / 12.0) + ((minutepos) / 60.0);
	vec4 macro_temp_6_a = vec4(arc(p, vec3((secondpos) * 6.28318, 0.05, 0.5)), 1.0, 0.0, 0.0);
	vec4 macro_temp_6_b = vec4(arc(p, vec3((minutepos) * 6.28318, 0.05, 0.35)), 0.0, 1.0, 0.0);
	float m = min((macro_temp_6_a).x, (macro_temp_6_b).x);
	macro_temp_6_a = (((macro_temp_6_a).x) == (m)) ? (macro_temp_6_a) : (macro_temp_6_b);
	macro_temp_6_b = vec4(arc(p, vec3((hourpos) * 6.28318, 0.05, 0.2)), 0.0, 0.0, 1.0);
	m = min((macro_temp_6_a).x, (macro_temp_6_b).x);
	return (((macro_temp_6_a).x) == (m)) ? (macro_temp_6_a) : (macro_temp_6_b);
}
void main() {
	vec2 p = (((((gl_FragCoord).xy) / ((iResolution).xy)) * 2.0) - 1.0) * (vec2(1.0, ((iResolution).y) / ((iResolution).x)));
	vec2 h = vec2(0.001, 0.0);
	vec3 mat = (distance_field(p)).yzw;
	gl_FragColor = vec4(vec3((clamp((- (((distance_field(p)).x) / (abs(length((vec2(((distance_field((p) + (h))).x) - ((distance_field((p) - (h))).x), ((distance_field((p) + ((h).yx))).x) - ((distance_field((p) - ((h).yx))).x))) / (2.0 * ((h).x))))))) * 200.0, 0.0, 1.0)) * (mat)), 1.0);
}
