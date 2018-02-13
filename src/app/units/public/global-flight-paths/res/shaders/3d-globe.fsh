#ifdef GL_ES
	precision lowp float;
#endif

const float av = 0.0;
const vec4 u_v4_ambient_color = vec4(av, av, av, 1.0);
const vec4 u_v4_diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 u_v4_specular_color = vec4(1.0, 1.0, 1.0, 1.0);
const float u_f_shininess = 55.0;

uniform mat4 u_m4_model;
uniform mat4 u_m4_model_view;
uniform mat4 u_m4_view_inv;
uniform sampler2D u_s2d_day;
uniform sampler2D u_s2d_night;
uniform sampler2D u_s2d_clouds;
uniform sampler2D u_s2d_normal_specular_map;

varying vec4 v_v4_view_pos;
varying vec4 v_v4_view_norm;
varying vec3 v_v3_light_dir;
varying vec3 v_v3_pos_ws;
varying vec2 v_v2_tex_coord;
varying vec3 v_v3_tangent;
varying vec3 v_v3_bitangent;
varying vec3 v_v3_normal;
varying vec2 v_v2_tex_coord2;

void main()
{
	vec4 v4_normal_specular = texture2D(u_s2d_normal_specular_map, v_v2_tex_coord);
	vec3 v3_base_nrm_ns = 2.0 * v4_normal_specular.rgb - 1.0;
	v3_base_nrm_ns.rg *= 0.5;
	//vec3 v3_cam_dir = normalize(v_v3_pos_ws - u_m4_view_inv[3].xyz);
	vec3 v3_view_norm = normalize(-v_v4_view_norm.xyz);
	
	// float dx = 10.0 * 0.00015;
	// float dy = 10.0 * 0.00015;
	// vec2 v2_tex_coord3 = vec2(v_v2_tex_coord2.x + dx, v_v2_tex_coord2.y + dy);
	vec4 color_day = texture2D(u_s2d_day, v_v2_tex_coord);
	vec4 color_night = texture2D(u_s2d_night, v_v2_tex_coord);
	vec4 color2 = texture2D(u_s2d_clouds, v_v2_tex_coord2);
	//vec4 color3 = texture2D(u_s2d_clouds, v2_tex_coord3);
	
	mat3 m3_vs_tf = mat3(u_m4_model_view);
	mat3 m3_ns_to_vs;
	
	m3_ns_to_vs[0] = m3_vs_tf * -v_v3_tangent;
	m3_ns_to_vs[1] = m3_vs_tf * v_v3_bitangent;
	m3_ns_to_vs[2] = m3_vs_tf * normalize(v_v3_normal);
	
	vec3 v3_normal_vs = normalize(m3_ns_to_vs * v3_base_nrm_ns);
	float fdf_i = max(0.0, dot(v3_view_norm, v_v3_light_dir));
	v3_view_norm = fdf_i * -v3_normal_vs + (1.0 - fdf_i) * v3_view_norm;
	vec3 v3_cam_dir = normalize(v_v4_view_pos.xyz);
	vec3 v3_h = normalize(v_v3_light_dir + v3_cam_dir);
	float fdf = dot(v3_view_norm, v_v3_light_dir);
	float f_diffuse_factor = max(0.0, fdf);
	f_diffuse_factor *= 2.0;
	float f_specular_factor = pow(max(0.0, dot(v3_view_norm, v3_h)) * sign(f_diffuse_factor), u_f_shininess);
	
	// float shadow_factor = (color3.r + color3.g + color3.b) / 3.0;
	// shadow_factor = shadow_factor * step(0.25, shadow_factor) * (0.5 - f_specular_factor);
	// shadow_factor = 1.0 - shadow_factor * 0.6;
	
	float alpha = color2.a * 1.25;
	float edge0 = 0.45;
	float edge1 = 0.75;
	float fff = fdf_i * 2.0;
	float nf = smoothstep(edge0, edge1, fdf_i);
	float nf2 = smoothstep(-0.35, edge0, fdf);
	nf2	+= color_night.a * (0.5 - 2.0 * fdf);
	
	vec4 ccc = (color_day * f_diffuse_factor * nf);// * shadow_factor;
	ccc += (1.0 - nf) * (1.75 * color_night * nf2);
	vec4 v4_diffuse_color = ccc + color2 * alpha * f_diffuse_factor;
	float f_alpha = 1.0;
	
	float f_mat_specular = v4_normal_specular.a * 0.5;
	vec4 v4_specular_color = u_v4_specular_color * f_mat_specular * f_specular_factor;
	
	v4_diffuse_color *= (u_v4_ambient_color + u_v4_diffuse_color);
	v4_diffuse_color += v4_specular_color;
	
	gl_FragColor = vec4(v4_diffuse_color.rgb, f_alpha);
}
