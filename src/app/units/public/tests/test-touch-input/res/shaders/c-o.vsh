attribute vec3 a_v3_position;

uniform mat4 u_m4_model_view_proj;

void main()
{
	gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
}
