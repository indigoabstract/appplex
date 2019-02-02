uniform mat4 u_m4_model;
uniform mat4 u_m4_model_view_proj;

attribute vec3 a_v3_position;
attribute vec2 a_v2_tex_coord;
attribute vec3 a_v3_normal;
attribute vec3 a_v3_tangent;
attribute vec3 a_v3_bitangent;

varying vec3 v_v3_pos_ws;
varying vec3 v_v3_normal_ws;
varying vec2 v_v2_tex_coord;
varying vec4 v_v4_color;
varying vec3 v_v3_tangent;
varying vec3 v_v3_bitangent;
varying vec3 v_v3_normal;
varying vec2 v_v2_sparkle_tex_coord;

void main()
{
	v_v3_pos_ws = (u_m4_model * vec4(a_v3_position, 1.0)).xyz;
	v_v3_normal_ws = normalize((u_m4_model * vec4(a_v3_normal, 0.0)).xyz);
	v_v2_tex_coord = a_v2_tex_coord;
	v_v4_color = vec4(1.0);
	v_v3_normal = a_v3_normal;
	v_v3_tangent = a_v3_tangent;
	v_v3_bitangent = a_v3_bitangent;
	vec3 np = normalize(a_v3_position);
	v_v2_sparkle_tex_coord = 3.71828 * (np.xy);

	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
}
