// http://glsl.heroku.com/e#13992.0
#ifdef GL_ES
precision mediump float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_mouse;
varying vec2 v_v2_tex_coord;

void main( void ) {

	vec2 position = ( v_v2_tex_coord) + u_v2_mouse / 4.0;

	float r = sin( u_v1_time + 0.0 + position.x ) * 0.5 + 0.5;
	float g = sin( u_v1_time + 1.0 + position.y ) * 0.5 + 0.5;
	float b = sin( u_v1_time + 2.0  + position.x) * 0.5 + 0.5;
	
	
	gl_FragColor = vec4(r,g,b,1.0);

}
