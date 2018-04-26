attribute vec3 a_v3_position;
attribute vec2 a_v2_tex_coord;

uniform mat4 u_m4_model_view_proj;

varying vec2 v_v2_tex_coord;

void main()
{
	v_v2_tex_coord = a_v2_tex_coord;
	
   	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
}