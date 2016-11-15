//@es #version 300 es
//@dt #version 330 core

layout(location=0) in vec3 a_v3_position;
layout(location=1) in vec4 a_iv4_color;
layout(location=2) in vec3 a_v3_normal;
layout(location=3) in vec2 a_v2_tex_coord;

uniform mat4 u_m4_model;
uniform mat4 u_m4_model_view;
uniform mat4 u_m4_model_view_proj;

smooth out vec4 v_v4_view_pos;
smooth out vec4 v_v4_world_pos;
smooth out vec4 v_v4_color;
smooth out vec4 v_v4_view_norm;
smooth out vec2 v_v2_tex_coord;
smooth out vec3 v_v3_world_coord;
smooth out vec3 v_v3_vx_color;

float random(vec3 n)
{
  return fract(sin(dot(n, vec3(25.1836, 99.476, 131.3856) + (1.0 / 12.67)))* 43758.5453);
}

float noise(vec3 pos)
{
	return fract(1111. * sin(111. * dot(pos, vec3(222222., 2222., 22.))));	
}

void main()
{
	v_v4_view_pos = u_m4_model_view * vec4(a_v3_position, 1.0);
	v_v4_world_pos = u_m4_model * vec4(a_v3_position, 1.0);
	v_v4_color = a_iv4_color;
	v_v4_view_norm = normalize(u_m4_model * vec4(a_v3_normal, 0.0));
	v_v2_tex_coord = a_v2_tex_coord;
	v_v3_world_coord = normalize(a_v3_position.xyz);
	float t = random(normalize(a_v3_position.xyz));
	v_v3_vx_color = vec3(t);
	
	gl_Position = u_m4_model_view_proj * vec4(a_v3_position+v_v3_vx_color*5.f, 1.0);
}
