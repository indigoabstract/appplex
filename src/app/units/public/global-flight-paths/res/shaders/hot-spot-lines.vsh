uniform mat4 u_m4_model_view;
uniform mat4 u_m4_projection;
uniform vec4 u_v4_start_vertex;
uniform vec3 u_v3_start_vertex_seg_dir;
uniform vec4 u_v4_end_vertex;
uniform vec3 u_v3_end_vertex_seg_dir;

attribute vec3 a_v3_position;
attribute vec3 a_v3_normal;
attribute vec2 a_v2_tex_coord;
attribute vec2 a_v2_tex_coord_1;

varying vec2 v_v2_tex_coord;

void main()
{
	vec3 position;
	vec3 direction;
	float v1_vertex_index = a_v2_tex_coord_1.x;
	
	if(v1_vertex_index < u_v4_start_vertex.w)
	{
		position = (u_m4_model_view * vec4(u_v4_start_vertex.xyz, 1.0)).xyz;
		direction = (u_m4_model_view * vec4(u_v3_start_vertex_seg_dir, 0.0)).xyz;
	}
	else if(v1_vertex_index > u_v4_end_vertex.w)
	{
		position = (u_m4_model_view * vec4(u_v4_end_vertex.xyz, 1.0)).xyz;
		direction = (u_m4_model_view * vec4(u_v3_end_vertex_seg_dir, 0.0)).xyz;
	}
	else
	{
		position = (u_m4_model_view * vec4(a_v3_position, 1.0)).xyz;
		direction = (u_m4_model_view * vec4(a_v3_normal, 0.0)).xyz;
	}
	
	float vsize = a_v2_tex_coord.s * a_v2_tex_coord_1.y;// * 10.5;
	float dotp = dot(normalize(position), normalize(direction));
	vec3 vect;
	
	// if(abs(dotp) > 0.985)
	// {
		// vect = position - (u_m4_model_view * vec4(0, 0, 0, 1.0)).xyz;
	// }
	// else
	{
		vect = cross(position, direction);
	}
	
	vect = normalize(vect);
	position = position + vect * vsize;
	v_v2_tex_coord = a_v2_tex_coord + vec2(0.5, 0.0);
	
	gl_Position = u_m4_projection * vec4(position, 1.0);
}
