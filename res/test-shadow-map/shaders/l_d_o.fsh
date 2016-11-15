#ifdef GL_ES
	precision lowp float;
#endif

const float av = 0.0;
vec4 u_v4_ambient_color = vec4(av, av, av, 1.0);
vec4 u_v4_diffuse_color = vec4(1.0, 1.0, 1.0, 1.0);

uniform vec3 u_v3_light_dir;
uniform sampler2D u_s2d_tex;

varying vec4 v_v4_view_pos;
varying vec4 v_v4_view_norm;
varying vec2 v_v2_tex_coord;

void main()
{
	vec3 v3_view_norm 			= normalize(-v_v4_view_norm.xyz);
	vec3 v3_light_dir 			= normalize(u_v3_light_dir);
	
	float f_diffuse_factor		= max(0.0, dot(v3_view_norm, v3_light_dir));
	vec4 v4_diffuse_color		= texture2D(u_s2d_tex, v_v2_tex_coord);
	float f_alpha 				= v4_diffuse_color.a;
	v4_diffuse_color 			*= (u_v4_ambient_color + u_v4_diffuse_color * f_diffuse_factor);
	
	gl_FragColor 				= vec4(v4_diffuse_color.rgb, f_alpha);
}
