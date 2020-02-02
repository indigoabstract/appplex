#ifdef GL_ES
	precision highp float;
#endif

uniform mat4 u_m4_model;
uniform mat4 u_m4_view_inv;

uniform sampler2D u_s2d_base_normal_map;
uniform sampler2D u_s2d_micro_flackes_normal_map;
uniform samplerCube u_scm_skybox;

uniform float u_v1_fresnel_exponent;
uniform float u_v1_brightness_factor;
uniform float u_v1_gloss_level;
uniform float u_v1_nrm_perturbation;
uniform float u_v1_flake_nrm_perturbation1;
uniform float u_v1_flake_nrm_perturbation2;

uniform vec4 u_v4_paint_front_color;
uniform vec4 u_v4_paint_middle_color;
uniform vec4 u_v4_paint_back_color;
uniform vec4 u_v4_flake_color;

varying vec3 v_v3_pos_ws;
varying vec2 v_v2_tex_coord;
varying vec3 v_v3_tangent;
varying vec3 v_v3_bitangent;
varying vec3 v_v3_normal;
varying vec2 v_v2_sparkle_tex_coord;

void main()
{
	vec3 v3_cam_dir = normalize(v_v3_pos_ws - u_m4_view_inv[3].xyz);
	vec3 v3_base_nrm_ns = 2.0 * texture2D(u_s2d_base_normal_map, v_v2_tex_coord).rgb - 1.0;
	vec3 v3_flake_nrm_ns = 2.0 * texture2D(u_s2d_micro_flackes_normal_map, v_v2_sparkle_tex_coord.xy).rgb - 1.0;
	vec3 v3_p_norm1_ns = u_v1_flake_nrm_perturbation1 * v3_flake_nrm_ns + u_v1_nrm_perturbation * v3_base_nrm_ns;
	vec3 v3_p_norm2_ns = u_v1_flake_nrm_perturbation2 * (v3_flake_nrm_ns + v3_base_nrm_ns);

	mat3 m3_ws_tf = mat3(u_m4_model);
	mat3 m3_ns_to_ws;
	
	m3_ns_to_ws[0] = m3_ws_tf * v_v3_tangent;
	m3_ns_to_ws[1] = m3_ws_tf * v_v3_bitangent;
	m3_ns_to_ws[2] = m3_ws_tf * normalize(v_v3_normal);
	
	vec3 v3_normal_ws = normalize(m3_ns_to_ws * v3_base_nrm_ns);
	float v3_dot_view_normal = clamp(dot(v3_normal_ws, -v3_cam_dir), 0.0, 1.0);
	vec3 v3_reflection_ws = normalize(reflect(v3_cam_dir, v3_normal_ws));
	float v1_cm_bias = u_v1_gloss_level;
	vec4 v4_reflected_color = textureCube(u_scm_skybox, v3_reflection_ws, v1_cm_bias);

	// if the cubemap alpha channel doesn't hold the pixel's light intensity, comment out the next line
	v4_reflected_color.rgb *= v4_reflected_color.a * u_v1_brightness_factor;
	
	vec3 v3_p_norm1_ws = normalize(m3_ns_to_ws * v3_p_norm1_ns);
	float v1_fresnel_fact1 = clamp(dot(v3_p_norm1_ws, -v3_cam_dir), 0.0, 1.0);
	vec3 v3_p_norm2_ws = normalize(m3_ns_to_ws * v3_p_norm2_ns);
	float v1_fresnel_fact2 = clamp(dot(v3_p_norm2_ws, -v3_cam_dir), 0.0, 1.0);
	float v1_fresnel_fact1_squared = v1_fresnel_fact1 * v1_fresnel_fact1;
	
	vec4 v4_c1 = v1_fresnel_fact1 * u_v4_paint_front_color;
	vec4 v4_c2 = v1_fresnel_fact1_squared * u_v4_paint_middle_color;
	vec4 v4_c3 = v1_fresnel_fact1_squared * v1_fresnel_fact1_squared * u_v4_paint_back_color;
	vec4 v4_c4 = pow(v1_fresnel_fact2, u_v1_fresnel_exponent) * u_v4_flake_color;
	vec4 v4_paint_layer_color = v4_c1 + v4_c2 + v4_c3 + v4_c4;
	float v1_reflected_color_weight = 1.0 - 0.5 * v3_dot_view_normal;
	vec3 v3_frag_color = v4_reflected_color.rgb * v1_reflected_color_weight + v4_paint_layer_color.rgb;

	gl_FragColor = vec4(v3_frag_color, 1.0);
	//gl_FragColor = vec4(v_v2_tex_coord, 0.0, 1.0);
}
