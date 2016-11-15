#ifdef GL_ES
	#extension GL_OES_standard_derivatives : enable
	precision lowp float;
#endif

uniform mat4 u_m4_model;
uniform sampler2D u_s2d_diffuse_map;
uniform sampler2D u_s2d_normal_map;
uniform sampler2D u_s2d_specular_map;
uniform vec3 u_v3_light_dir;

varying vec3 v_v3_pos_ws;
varying vec3 v_v3_normal_ms;
varying vec3 v_v3_normal_ws;
varying vec3 v_v3_tangent_ns;
varying vec3 v_v3_bitangent_ns;
varying vec2 v_v2_tex_coord;

vec3 GetNormal(vec3 N, vec3 C, sampler2D normal_map, vec2 uv);

void main()
{
	vec4 v4_diffuse_color = texture2D(u_s2d_diffuse_map, v_v2_tex_coord);
	vec3 v3_normal_val = texture2D(u_s2d_normal_map, v_v2_tex_coord).rgb;
	v3_normal_val = 2.0 * v3_normal_val - 1.0;
	vec4 v4_specular_val = texture2D(u_s2d_specular_map, v_v2_tex_coord);
	vec3 v3_normal_ws = normalize(v_v3_normal_ws);
	mat3 ts_to_ws;
	mat3 wm = mat3(u_m4_model);
	
	ts_to_ws[0] = wm * v_v3_tangent_ns;
	ts_to_ws[1] = wm * v_v3_bitangent_ns;
	ts_to_ws[2] = wm * v_v3_normal_ms;
	
	vec3 v3_normal_val_ws = normalize(ts_to_ws * v3_normal_val);
	//v3_normal_val_ws = GetNormal(v_v3_normal_ws, v_v3_pos_ws, u_s2d_normal_map, v_v2_tex_coord);
	float f_diffuse_fact = clamp(dot(-u_v3_light_dir, v3_normal_val_ws), 0.0, 1.0);
	
	v4_diffuse_color *= f_diffuse_fact;
	gl_FragColor = vec4(v4_diffuse_color.rgb, 1.0);
}

vec3 GetNormal(vec3 N, vec3 C, sampler2D normal_map, vec2 uv)
{
	vec3 dpx = dFdx(C);
	vec3 dpy = dFdy(C);
	vec2 dtx = dFdx(uv);
	vec2 dty = dFdy(uv);
	
	vec3 T = normalize(dpy * dtx.y - dpx * dty.y);
	vec3 B = cross(T, N);
	vec3 normal = texture2D(normal_map, uv).xyz * 2.0 - 1.0;
	
	return normalize(mat3(T, B, N) * normal);
}
