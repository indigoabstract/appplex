#ifdef GL_ES
	precision lowp float;
#endif

uniform vec4 u_v4_color;

void main()
{
	gl_FragColor = u_v4_color;
}
