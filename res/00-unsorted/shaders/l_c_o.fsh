#ifdef GL_ES
	precision lowp float;
#endif

uniform vec4 u_v4_color;
uniform vec3 u_v3_light_dir;

varying vec4 v_v4_world_pos;
varying vec4 v_v4_color;

void main()
{
	vec3 v3_normal = normalize(cross(dFdx(v_v4_world_pos.xyz), dFdy(v_v4_world_pos.xyz)));
	vec3 v3_light_dir = normalize(u_v3_light_dir);
	float v1_diffuse_factor = max(0.05, dot(-v3_normal, v3_light_dir));
	
	gl_FragColor = vec4(v_v4_color.rgb * v1_diffuse_factor, v_v4_color.a);
}
