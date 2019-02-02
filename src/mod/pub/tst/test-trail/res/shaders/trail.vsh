attribute vec3 a_v3_position;
attribute vec3 a_v3_normal;
attribute vec2 a_v2_tex_coord;

varying float v_v1_alpha;

uniform mat4 u_m4_model;
uniform mat4 u_m4_view;
uniform mat4 u_m4_projection;
uniform vec3 u_v3_positions[256];
uniform float u_v1_total_positions;
uniform float u_v1_line_thickness;

const float v1_line_thickness_scale = 1.;

void main()
{
	int pos_index = int(a_v3_position.x);
	float side_sign = a_v2_tex_coord.x;
	
	vec4 view_space_line_direction;
	
	if(pos_index > 0)
	{
		view_space_line_direction = u_m4_view * u_m4_model * vec4(u_v3_positions[pos_index] - u_v3_positions[pos_index - 1], 0.);
	}
	else
	{
		view_space_line_direction = u_m4_view * u_m4_model * vec4(u_v3_positions[1] - u_v3_positions[0], 0.);
	}
	
	v_v1_alpha = 1.;//pos_index / u_v1_total_positions;
	
	vec2 v2_vs_line_dir_perp = normalize(vec2(-view_space_line_direction.y, view_space_line_direction.x));
	
	vec3 v3_position = u_v3_positions[pos_index];
	vec4 v4_vs_pos = u_m4_view * u_m4_model * vec4(v3_position, 1);
	// offset the 2 ends of the quad along the perpendicular to the line direction (in viewspace)
	v4_vs_pos.xy += side_sign * v2_vs_line_dir_perp * u_v1_line_thickness * v1_line_thickness_scale;
	
	// this line keeps a_v3_normal in the shader (the engine can't handle missing attributes at the moment)
	v4_vs_pos.xy += a_v3_normal.xy * 0.000001;
	
	gl_Position = u_m4_projection * v4_vs_pos;
}
