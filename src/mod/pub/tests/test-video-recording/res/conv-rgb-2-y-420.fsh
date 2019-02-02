#ifdef GL_ES
	precision lowp float;
#endif
			
varying vec2 v_v2_tex_coord;
uniform sampler2D u_s2d_tex;

float rgb_2_y(vec3 c)
{
	float result = (0.257 * c.r) + (0.504 * c.g) + (0.098 * c.b) + 0.0625;
	
	return result;
}

void main()
{
    vec3 v3_color = texture2D(u_s2d_tex, v_v2_tex_coord).rgb;
	
	gl_FragColor.r = rgb_2_y(v3_color);
}
