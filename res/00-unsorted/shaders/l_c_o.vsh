attribute vec3 a_v3_position;
attribute vec4 a_iv4_color;

uniform mat4 u_m4_model;
uniform mat4 u_m4_model_view_proj;

varying vec4 v_v4_world_pos;
varying vec4 v_v4_color;

void main()
{
	v_v4_world_pos = u_m4_model * vec4(a_v3_position, 1.0);
	v_v4_color = a_iv4_color;
	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
}
