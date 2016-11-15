// http://glsl.heroku.com/e#15247.0
#ifdef GL_ES
precision mediump float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

const vec2 gravity = vec2(0, -.3);

// thank you for this function, anonymous person on the interwebs
float
rand(vec2 co)
{
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233)))*43758.5453);
}

float
rand(float from, float to, vec2 co)
{
	return from + rand(co)*(to - from);
}

void main(void)
{
	float scale = 1.5/max(u_v2_dim.x, u_v2_dim.y);
	vec2 coord = v_v2_tex_coord*u_v2_dim*scale;
	
	vec2 origin = vec2(.5*u_v2_dim.x*scale, 0.0);
	vec4 color = vec4(0.);

	for (float i = 1.; i < 2.; i++) {
		float period = rand(1.5, 2.5, vec2(i, 0.));

		float t = u_v1_time - period*rand(vec2(i, 1.));

		float particle_time = mod(t, period);
		float index = ceil(t/period);

		vec2 speed = vec2(rand(-.5, .5, vec2(index*i, 3.)), rand(.5, 1., vec2(index*i, 4.)));
		vec2 pos = origin + particle_time*speed + gravity*particle_time*particle_time;

		float threshold = .7*period;

		float alpha;
		if (particle_time > threshold)
			alpha = 1.0 - (particle_time - threshold)/(period - threshold);
		else
			alpha = 1.0;

		vec4 particle_color = vec4(rand(vec2(i*index, 2.0)), rand(vec2(i*index, 1.5)), rand(vec2(i*index, 1.2)), 1.0);

		float angle_speed = rand(-4.0, 4.0, vec2(index*i, 5.0));
		float angle = atan(pos.y - coord.y, pos.x - coord.x) + angle_speed*u_v1_time;

		float radius = rand(.09, .05, vec2(index*i, 2.));

		float dist = radius + sin(angle)*radius;

		color += alpha * ((1.0 - smoothstep(dist, dist + .05, distance(coord, pos))) + 
					              (1.0 - smoothstep(0.01, 0.02, distance(coord, pos))) +
					              (1.0 - smoothstep(radius * 0.1, radius * 0.9, distance(coord, pos)))) * particle_color;
	}

	gl_FragColor = color;
}
