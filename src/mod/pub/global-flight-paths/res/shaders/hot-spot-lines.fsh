#ifdef GL_ES
	precision lowp float;
#endif

uniform sampler2D u_s2d_tex;
uniform vec4 u_vertex_color;

varying vec2 v_v2_tex_coord;

void main()
{
	vec4 v4_diff_color = texture2D(u_s2d_tex, v_v2_tex_coord);
	//v4_diff_color.a -= v4_diff_color.a / 6.0;
	v4_diff_color.a *= 0.75;
	gl_FragColor = v4_diff_color;// * u_vertex_color;
}
