// https://www.shadertoy.com/view/MsBGWd
#define TAU 6.283185307179586
#define UNIT 1.3

float scale(float l0, float r0, float l1, float r1, float x) {
	return (x - l0) / (r0 - l0) * (r1 - l1) + l1;
}

vec2 shear(float theta, vec2 p) {
	return vec2(p.x - p.y / tan(theta), p.y / sin(theta));
}

vec2 unshear(float theta, vec2 p) {
	float y = p.y * sin(theta);
	float x = p.x + y / tan(theta);
	return vec2(x, y);	
}

float rand(vec2 p){
	return fract(sin(dot(p.xy, vec2(1.3295, 4.12))) * 493022.1);
}

float timestep(float duration, float t) {
	return floor(t / duration) * duration;
}

vec2 polar(vec2 p) {
	return vec2(atan(p.y, p.x), length(p));
}

vec2 cartesian(vec2 p) {
	return vec2(p.y * cos(p.x), p.y * sin(p.x))	;
}

vec3 eyes(float t) {
	vec2 p0 = 2.0*(gl_FragCoord.xy - 0.5 * iResolution.xy) / iResolution.xx;
	
	float unit = UNIT;
	float d_step0 = 1.1544 * unit;
	float d_step1 = 1.823 * unit;
	float d_step2 = 2.32 * unit;
	float d_step3 = 2.9757 * unit;
	float d_step4 = 1.21 * unit;
	float d_step5 = 0.93354 * unit;
	
	float t_step0 = timestep(d_step0, t);
	vec2 p0_rot = cartesian(polar(p0) + vec2(scale(0.0, 1.0, 0.0, TAU, rand(vec2(0.0, sqrt(t_step0)))), 0.0));
	vec2 p0_step = p0_rot + vec2(
		scale(0.0, 1.0, -1.0, 1.0, rand(vec2(t_step0, 0.0))),
		scale(0.0, 1.0, -1.0, 1.0, rand(vec2(0.0, t_step0)))
	);
	float theta = TAU/4.0 + scale(0.0, 1.0, -1.0, 1.0, rand(vec2(0.0, timestep(d_step1, t))));

	float k_p1 = scale(0.0, 1.0, 2.0, 5.0, rand(vec2(timestep(d_step2, t), 0.0)));
	vec2 p1 = k_p1 * (p0_step /*+ 0.125*/);
		
	vec2 p2 = shear(theta, p1);
	
	float d_move = 0.4;
	vec2 p_c = floor(p2) + 0.5 + scale(0.0, 1.0, -d_move, d_move, rand(vec2(timestep(d_step3, t), 0.0)));
	vec2 p3 = unshear(theta, p_c);
		
	float radius = scale(0.0, 1.0, 0.3, 0.6, rand(vec2(-42.0, timestep(0.21 * unit, t))));//0.25;
	float rings = floor(scale(0.0, 1.0, 1.0, 4.0, rand(vec2(0.0, timestep(d_step4, t)))));
	float dist = distance(p3, p1);
	float ring_i = floor(dist/radius * rings);
	float ring_i2 = floor(dist / radius * 2.0 * rings);
	float ring_pos = fract(dist / radius * rings);
	float ring_pos2 = fract(dist / radius * 2.0 * rings);
	float r_pupil = radius / scale(0.0, 1.0, 1.5, 2.0, rand(vec2(timestep(0.322*unit, t), 0.0)));
	
	bool in_pupil = dist < r_pupil;
	bool in_iris = dist < radius;
	
	float bright = 1.0 - 0.75 * ring_i/rings * radius;
	float k_light = scale(0.0, 1.0, 0.76, 1.25, rand(vec2(-42.0, timestep(0.267*unit, t))));
	float light = k_light * bright * scale(0.0, 1.0, 0.5, 1.0, ring_pos);
	vec3 color = vec3(light, light, light);
	if(in_pupil) {
		color = vec3(0.0, 0.0, 0.0);
	} else if(in_iris) {
		vec3 iris0 = vec3(
			scale(-1.0, 1.0, 0.2, 0.96, sin(timestep(0.2*unit, 0.6654*t))),
			scale(0.0, 1.0, 0.0, 0.6, rand(floor(p2) * vec2(timestep(0.33*unit, .5*t + 53.0*floor(p2.x+p2.y)), -3211.1))),
			scale(-1.0, 1.0, 0.1, 0.7, sin(timestep(0.115*unit, 0.533*t)))
		);

		vec3 iris1 = iris0 * 0.7;
		
		color = mix(iris0, iris1, ring_pos2);
	}
	color *= scale(0.0, 1.0, 0.3, 1.0, rand(floor(p2) + vec2(timestep(1.0*unit, t), 0.1222)));
	
	return color;
}

void main(void)
{
	float t_0 = iGlobalTime;
	float t_1 = t_0 + UNIT*1.7;
	float mod_offset = 0.0;
	float mod0 = fract(iGlobalTime / UNIT);
	float mod_t = scale(-1.0, 1.0, 0.3, 0.7, sin(5.55 * iGlobalTime));
	
	vec3 color = eyes(t_0);
	if(rand(gl_FragCoord.xy + vec2(0.0, timestep(0.125 * UNIT, iGlobalTime))) < mod_t) {
		color = eyes(t_1);	
	}
	
	gl_FragColor = vec4(color, 1.0);
}
