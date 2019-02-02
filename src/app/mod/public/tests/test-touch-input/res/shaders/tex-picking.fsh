#ifdef GL_ES
	precision lowp float;
#endif
			
varying vec2 v_v2_tex_coord;
uniform sampler2D u_s2d_tex;
uniform float u_v1_z_pos;

void main()
{	
    vec4 v4_color = texture2D(u_s2d_tex, v_v2_tex_coord);
	float v1_f = (v4_color.r+v4_color.g+v4_color.b) * v4_color.a;
	float v1_val = step(0.01, v1_f);
	
	if(v1_val == 0.)
	{
		discard;
	}
	
	gl_FragColor.r = v1_val * u_v1_z_pos;
}
