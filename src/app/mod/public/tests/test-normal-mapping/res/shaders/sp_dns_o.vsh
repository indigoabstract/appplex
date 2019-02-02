uniform mat4 u_m4_model_view_proj;
uniform mat4 u_m4_model;

attribute vec3 a_v3_position;
attribute vec3 a_v3_normal;
attribute vec2 a_v2_tex_coord;
attribute vec3 a_v3_tangent;
attribute vec3 a_v3_bitangent;

varying vec3 v_v3_pos_ws;
varying vec3 v_v3_normal_ms;
varying vec3 v_v3_normal_ws;
varying vec3 v_v3_tangent_ns;
varying vec3 v_v3_bitangent_ns;
varying vec2 v_v2_tex_coord;

void main()
{
	v_v3_pos_ws = (u_m4_model * vec4(a_v3_position, 1.0)).xyz;
	v_v3_normal_ms = a_v3_normal;
	v_v3_normal_ws = normalize((u_m4_model * vec4(a_v3_normal, 1.0)).xyz);
	v_v3_tangent_ns = a_v3_tangent;
	v_v3_bitangent_ns = a_v3_bitangent;
	v_v2_tex_coord = a_v2_tex_coord;	
	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
}
