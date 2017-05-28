#ifdef GL_ES
	precision lowp float;
#endif

uniform vec4 u_v4_color;

varying vec3 v_v3_normal_ws;

vec3 v3_light_dir = vec3(1, 1, 1);

void main()
{
	vec3 v3_normal_ws = normalize(v_v3_normal_ws);
	float v1_dotp = clamp(dot(v3_normal_ws, v3_light_dir), 0., 1.);
	v1_dotp = max(v1_dotp, 0.3);
	vec3 v3_color = u_v4_color.rgb * v1_dotp;
	
	gl_FragColor = vec4(v3_color, u_v4_color.a);
}
