#ifdef GL_ES
	precision lowp float;
#endif
			
varying vec2 v_v2_tex_coord;
uniform sampler2D u_s2d_tex;

void main()
{	
    vec4 v4_color = texture2D(u_s2d_tex, v_v2_tex_coord);
	gl_FragColor = v4_color;
	//float z = gl_FragCoord.z;
	//gl_FragColor = vec4(vec3(z), 1.);
}
