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
varying vec3 v_v3_normal_ws;
varying vec2 v_v2_tex_coord;
varying vec3 v_v3_tangent;
varying vec3 v_v3_bitangent;
varying vec3 v_v3_normal;
varying vec2 v_v2_sparkle_tex_coord;


void main()
{
	vec3 v3_base_color = vec3(0.2, 0.1, 1.0);
	vec3 v3_cam_dir_ws = normalize(v_v3_pos_ws - u_m4_view_inv[3].xyz);
	vec3 v3_light_dir_ws = -v3_cam_dir_ws;
	mat3 m3_ws_tf = mat3(u_m4_model);
	mat3 m3_ns_to_ws;
	
	m3_ns_to_ws[0] = m3_ws_tf * v_v3_tangent;
	m3_ns_to_ws[1] = m3_ws_tf * v_v3_bitangent;
	m3_ns_to_ws[2] = m3_ws_tf * normalize(v_v3_normal);
	
	vec3 v3_base_nrm_ns = 2.0 * texture2D(u_s2d_base_normal_map, v_v2_tex_coord).rgb - 1.0;
	vec3 v3_flake_nrm_ns = 2.0 * texture2D(u_s2d_micro_flackes_normal_map, v_v2_sparkle_tex_coord.xy).rgb - 1.0;
	vec3 v3_normal_ws = normalize(m3_ns_to_ws * v3_base_nrm_ns);
	v3_normal_ws = v_v3_normal_ws;
	
	vec3 v3_c1 = v3_base_color * 0.5 * (pow(max(0.0, dot(v3_normal_ws, v3_light_dir_ws)), 2.0) + 0.2);
	vec3 v3_half_view = normalize(v3_light_dir_ws - v3_cam_dir_ws);
	float v3_dot_half_view_normal = dot(v3_half_view, v3_normal_ws);
	vec3 v3_spec_color = pow(max(0.0, v3_dot_half_view_normal), 50.0) * 10.0 * vec3(1.0);
	float v3_dot_view_normal = dot(v3_cam_dir_ws, v3_normal_ws);
	vec3 v3_reflection_ws = reflect(v3_cam_dir_ws, v3_normal_ws);
	vec4 v4_refl_color = textureCube(u_scm_skybox, v3_reflection_ws, 2.0);
	float v1_intensity = (v4_refl_color.r + v4_refl_color.g + v4_refl_color.b) / 3.0;
	float v1_gloss_angle = v3_reflection_ws.y - 0.075;
	
	vec3 v3_gloss_color = mix(vec3(1.0), vec3(0.0), fract(v1_gloss_angle * 0.17));
	v3_gloss_color += v4_refl_color.rgb * v1_intensity * 2.75;
	v3_spec_color += v3_gloss_color;

	float v1_mix_fact = pow(1.0 + v3_dot_view_normal, 5.0);
	v1_mix_fact = mix(0.2, 1.0, v1_mix_fact);
	
	v3_c1 = mix(v3_c1, v3_base_color * v3_spec_color, v1_mix_fact);
	
	v3_spec_color = pow(max(0.0, v3_dot_half_view_normal), 1000.0) * 25.0 * vec3(1.0);
	v3_spec_color += v3_gloss_color;
	
	v1_mix_fact = pow(min(1.0, 1.0 + v3_dot_view_normal), 5.0);
	v1_mix_fact = mix(0.03, 1.0, v1_mix_fact);
	//v1_mix_fact = smoothstep(0.1, 0.9, v1_mix_fact);

	vec3 v3_p_norm1_ns = u_v1_flake_nrm_perturbation2 * (v3_flake_nrm_ns + v3_base_nrm_ns);
	vec3 v3_p_norm1_ws = normalize(m3_ns_to_ws * v3_p_norm1_ns);
	float v1_fresnel_fact1 = clamp(dot(v3_p_norm1_ws, -v3_cam_dir_ws), 0.0, 1.0);
	float v1_flake_limit = pow(max(0.0, v3_dot_half_view_normal), 750.0) * 25.0;
	vec3 v3_c2 = pow(v1_fresnel_fact1, u_v1_fresnel_exponent * 100.0) * u_v4_flake_color.rgb * v1_flake_limit;
	
	vec3 v3_frag_color = mix(v3_c1, v3_spec_color, v1_mix_fact) + v3_c2 * 0.25;

	gl_FragColor = vec4(v3_frag_color, 1.0);
	//gl_FragColor = vec4(abs(v_v3_bitangent), 1.0);
}
