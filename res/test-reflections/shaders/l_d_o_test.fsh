#ifdef GL_ES
	precision lowp float;
#endif

const float u_f_shininess = 295.0;
const float av = 0.1;
vec4 u_v4_ambient_color = vec4(av, av, av, 1.0);
vec4 u_v4_diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);
const vec4 u_v4_specular_color = vec4(1.0, 1.0, 1.0, 1.0);

uniform mat4 u_m4_model_view;
uniform mat4 u_m4_view_inv;
uniform mat4 u_m4_view;
uniform vec3 u_v3_light_dir;
uniform sampler2D u_s2d_tex;
uniform samplerCube u_scm_skybox;

varying vec3 v_v3_pos;
varying vec3 v_v3_norm;
varying vec4 v_v4_view_pos;
varying vec4 v_v4_view_norm;
varying vec2 v_v2_tex_coord;

void main()
{
	vec3 v3_norm = normalize(-v_v3_norm);
	float f_diffuse_factor = clamp(dot(-u_v3_light_dir, v3_norm), 0.0, 1.0);
	
	vec3 v3_cam_dir = normalize(v_v3_pos - u_m4_view_inv[3].xyz);
	float f_f = clamp(dot(v3_norm, v3_cam_dir) * 1.25, 0.0, 1.0);
	float f_rf = 1.0 - f_f;
	vec4 c1 = vec4(0.15, 0.0, 0.25, 1.0);
	vec4 c2 = vec4(0.0, 0.35, 0.0, 1.0);
	vec4 c = f_rf * c1 + f_f * c2;
	//c = vec4(f_f,f_f,f_f,1.0);
	//vec4 v4_diffuse_color = texture2D(u_s2d_tex, v_v2_tex_coord);
	vec4 v4_diffuse_color = vec4(c.rgb, 1.0);
	float f_alpha = v4_diffuse_color.a;
	
	vec3 v3_reflected_light = normalize(reflect(u_v3_light_dir, v3_norm));
	vec3 v3_reflected_view = normalize(reflect(v3_cam_dir, v3_norm));
	float f_v = dot(v3_cam_dir, v3_reflected_light);
	float f_specular_factor = pow(max(0.0, f_v), u_f_shininess);
	float f_mat_specular = 0.5;
	vec4 v4_specular_color = u_v4_specular_color * f_mat_specular * f_specular_factor;
	vec4 v4_reflected_color = textureCube(u_scm_skybox, v3_reflected_view);
	//v4_diffuse_color = (1.0 - f_rf) * v4_diffuse_color + f_rf * v4_reflected_color;
	v4_diffuse_color += f_rf * v4_reflected_color * 0.75;
	
	v4_diffuse_color *= (u_v4_ambient_color + u_v4_diffuse_color * f_diffuse_factor);
	v4_diffuse_color += v4_specular_color;
	
	//gl_FragColor = vec4(v4_diffuse_color.rgb, f_alpha);
	gl_FragColor = v4_reflected_color;
}
