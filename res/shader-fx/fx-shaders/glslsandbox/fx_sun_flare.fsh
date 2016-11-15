// http://glsl.heroku.com/e#6065.0
//  fork'd by @dennishjorth. Thanks to the original (#6054.0).

#ifdef GL_ES
precision highp float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_mouse;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

// Singularity -- Using this for other applications. Makes a good flare image. Thanks to whoever made this originally!

#define BLADES 6.0
#define BIAS 0.2
#define SHARPNESS 5.0
#define COLOR 0.4, 0.65, 0.82
#define BG 0.34, 0.52, 0.76

void main( void ) {

	vec2 sunpos = vec2(0.);//(u_v2_mouse - 0.5) / vec2(u_v2_dim.y/u_v2_dim.x,1.0);
	sunpos.x += cos(u_v1_time*0.5)*cos(u_v1_time*0.1)*0.3;
	sunpos.y += sin(u_v1_time*0.5)*cos(u_v1_time*0.1)*0.3;
	vec2 position = (( v_v2_tex_coord ) - vec2(0.5)) / vec2(u_v2_dim.y/u_v2_dim.x,1.0);
	vec2 t = position.yx - sunpos.yx;
	float alpha = 1.;//sunpos.x * 0.5 + sin(u_v1_time * 0.1) * 0.02;
	//t = mat2(cos(alpha), -sin(alpha), sin(alpha), cos(alpha)) * t;
	
	float blade = clamp(pow(sin(u_v1_time*0.7+atan(t.x, t.y)*(BLADES+2.0))+BIAS, SHARPNESS), 0.0, 1.0);

	float dist = .2 / distance(sunpos, position) * 0.075;
	float dist2 = 2.5+0.5*cos(u_v1_time*0.3);// - distance(vec2(0.0), sunpos);
	dist2 = pow(dist2, 3.5);
	dist *= dist2*dist2*0.02;

	vec3 color = vec3(0.0);
	//color = mix(vec3(0.34, 0.5, 1.0), vec3(0.0, 0.5, 1.0), (position.y + 1.0) * 0.5);

	color += vec3(0.95+blade, 0.65, 0.35) * dist;
	color += vec3(0.95, 0.45, 0.30) * min(1.0, blade *0.7) * dist*1.;

	gl_FragColor = vec4( color.x,color.y,color.z, 1.0 );
}
