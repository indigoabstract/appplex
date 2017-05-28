#ifdef GL_ES
	precision lowp float;
#endif

varying float v_v1_alpha;

uniform vec4 u_v4_color;

void main()
{
	gl_FragColor = u_v4_color;
	gl_FragColor.a *= v_v1_alpha;
}
