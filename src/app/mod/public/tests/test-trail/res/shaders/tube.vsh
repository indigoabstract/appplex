attribute vec3 a_v3_position;
attribute vec3 a_v3_normal;
attribute vec2 a_v2_tex_coord;

varying vec3 v_v3_normal_ws;

uniform mat4 u_m4_model;
uniform mat4 u_m4_model_view_proj;

void main()
{
	vec4 v4_norm_ws = u_m4_model * vec4(a_v3_normal, 0.);
	v_v3_normal_ws = v4_norm_ws.xyz;
	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.);
	gl_Position.z -= 0.1;
}
