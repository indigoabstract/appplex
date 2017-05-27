#ifdef GL_ES
	precision lowp float;
#endif
			
varying vec2 v_v2_tex_coord;
uniform sampler2D u_s2d_tex;

void main()
{	
    gl_FragColor = texture2D(u_s2d_tex, v_v2_tex_coord);
}
