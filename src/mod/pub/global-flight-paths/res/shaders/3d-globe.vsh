uniform mat4 u_m4_model;
uniform mat4 u_m4_view;
uniform mat4 u_m4_model_view;
uniform mat4 u_m4_model_view_proj;
uniform vec3 u_v3_light_dir;
const float u_f_tex_x_offset =  0.0;

attribute vec3 a_v3_position;
attribute vec2 a_v2_tex_coord;
attribute vec3 a_v3_normal;
attribute vec3 a_v3_tangent;
attribute vec3 a_v3_bitangent;

varying vec3 v_v3_pos_ws;
varying vec4 v_v4_view_pos;
varying vec4 v_v4_view_norm;
varying vec3 v_v3_light_dir;
varying vec2 v_v2_tex_coord;
varying vec3 v_v3_tangent;
varying vec3 v_v3_bitangent;
varying vec3 v_v3_normal;
varying vec2 v_v2_tex_coord2;

void main()
{
	v_v3_pos_ws = (u_m4_model * vec4(a_v3_position, 1.0)).xyz;
	v_v4_view_pos = u_m4_model_view * vec4(a_v3_position, 1.0);
	v_v4_view_norm = normalize(u_m4_model_view * vec4(a_v3_normal, 0.0));
	v_v3_light_dir = normalize((u_m4_view * vec4(u_v3_light_dir.xyz, 0.0)).xyz);
	v_v2_tex_coord = a_v2_tex_coord;
	v_v3_normal = a_v3_normal;
	v_v3_tangent = a_v3_tangent;
	v_v3_bitangent = a_v3_bitangent;
	v_v2_tex_coord2 = vec2(a_v2_tex_coord.x + u_f_tex_x_offset, a_v2_tex_coord.y);
	
	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
}
