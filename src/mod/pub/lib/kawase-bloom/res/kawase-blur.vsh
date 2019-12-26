//@es #version 300 es
//@dt #version 330 core

layout(location = 0) in vec3 a_v3_position;
layout(location = 1) in vec2 a_v2_tex_coord;

uniform mat4 u_m4_model_view_proj;

smooth out vec2 v_v2_tex_coord;

void main()
{ 	 	
	v_v2_tex_coord = a_v2_tex_coord;
	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
}
