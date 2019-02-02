uniform mat4 u_m4_model_view;
uniform mat4 u_m4_projection;

attribute vec3 a_v3_position;
attribute vec2 a_v2_tex_coord;
varying vec2 v_v2_tex_coord;

void main()
{
   	//gl_Position = u_m4_projection * u_m4_model_view * vec4(a_v3_position, 1.0);
   	gl_Position = vec4(a_v3_position, 1.0);
	v_v2_tex_coord = a_v2_tex_coord;
}
