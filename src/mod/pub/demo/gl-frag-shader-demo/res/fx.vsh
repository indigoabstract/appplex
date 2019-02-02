attribute vec3 a_v3_position;
attribute vec2 a_v2_tex_coord;

uniform mat4 u_m4_model_view;
uniform mat4 u_m4_projection;
varying vec2 v_v2_tex_coord;
// NEW: This 'varying' vec2 indicates the position of the current fragment on the virtual surface.
varying vec2 surfacePosition;

void main()
{
	//vec4 v4_cam_space = u_m4_model_view * vec4(a_v3_position, 1.0);
   	//gl_Position = u_m4_projection * v4_cam_space;
   	gl_Position = vec4(a_v3_position, 1.0);
	v_v2_tex_coord = a_v2_tex_coord;
	//surfacePosition = v_v2_tex_coord - vec2(0.5);
	surfacePosition = v_v2_tex_coord * vec2(2.) - vec2(1.);
}