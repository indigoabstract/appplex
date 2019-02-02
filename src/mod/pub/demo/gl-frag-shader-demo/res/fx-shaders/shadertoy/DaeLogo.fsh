// https://www.shadertoy.com/view/MdlSD8
vec3 rotate(vec3, float);
float frame(vec2);
vec2 cart_polar(vec2);
vec2 polar_cart(vec2);
float p_circle(vec2, vec3);
vec3 cwarp(vec3);
vec3 rotate(vec3 c, float a) {
	a = (a) * 0.0174532777778;
	float ca = cos(a);
	float sa = sin(a);
	return vec3((((c).x) * (ca)) - (((c).y) * (sa)), (((c).y) * (ca)) + (((c).x) * (sa)), (c).z);
}
float frame(vec2 p) {
	return max(max(p_circle(p, vec3(0.0, 0.0, 0.6)), - (max(max(max(max(p_circle(p, vec3(0.0, 0.0, 0.4)), - (max(max(p_circle(p, rotate(vec3(0.13625, -0.04, 0.52221283975), 240.0)), p_circle(p, rotate(vec3(-0.25375, 0.135, 0.464630229322), 240.0))), - (p_circle(p, rotate(vec3(-0.2925, -0.10125, 0.46492270863), 240.0)))))), - (max(max(p_circle(p, rotate(vec3(0.13625, -0.04, 0.52221283975), 120.0)), p_circle(p, rotate(vec3(-0.25375, 0.135, 0.464630229322), 120.0))), - (p_circle(p, rotate(vec3(-0.2925, -0.10125, 0.46492270863), 120.0)))))), - (max(max(p_circle(p, rotate(vec3(0.13625, -0.04, 0.52221283975), 0.0)), p_circle(p, rotate(vec3(-0.25375, 0.135, 0.464630229322), 0.0))), - (p_circle(p, rotate(vec3(-0.2925, -0.10125, 0.46492270863), 0.0)))))), - (max(max(max(p_circle(p, vec3(0.0, 0.18375, 0.295047665641)), p_circle(p, rotate(vec3(0.0, 0.18375, 0.295047665641), 120.0))), p_circle(p, rotate(vec3(0.0, 0.18375, 0.295047665641), 240.0))), - (p_circle(p, vec3(0.0, 0.0, 0.0434)))))))), 0.0);
}
vec2 cart_polar(vec2 p) {
	return vec2(atan((p).y, (p).x), length(p));
}
vec2 polar_cart(vec2 p) {
	return (vec2(cos((p).x), sin((p).x))) * ((p).y);
}
float p_circle(vec2 p, vec3 c) {
	c = cwarp(c);
	return (length(((c).xy) - (p))) - ((c).z);
}
void main() {
	vec2 p = ((((((gl_FragCoord).xy) / ((iResolution).xy)) * 2.0) - 1.0) * (vec2(1.0, - (((iResolution).y) / ((iResolution).x))))) * 1.2;
	vec2 h = vec2(0.0001, 0.0);
	float d = clamp(((abs(frame(p))) / (length((vec2((frame((p) + (h))) - (frame((p) - (h))), (frame((p) + ((h).yx))) - (frame((p) - ((h).yx))))) / (2.0 * ((h).x))))) * 100.0, 0.0, 1.0);
	gl_FragColor = vec4(d, d, d, 1.0);
}
vec3 cwarp(vec3 c) {
	float t = 1.0 - (pow(clamp(((abs((mod(iGlobalTime, 8.0)) - 4.0)) - 1.0) / 1.5, 0.0, 1.0), 3.0));
	return vec3(polar_cart((cart_polar(((c).xy) * (1.0 - (t)))) + (vec2(((t) * 3.14159) * -1.0, 0.0))), ((c).z) * (((t) * 2.5) + 1.0));
}
