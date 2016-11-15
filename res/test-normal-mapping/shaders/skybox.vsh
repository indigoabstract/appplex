//@es #version 300 es
//@dt #version 330 core

layout(location=0) in vec3 a_v3_position; //object space vertex position

uniform mat4 u_m4_model_view_proj; //combined modelview projection matrix

//output to fragment shader
smooth out vec3 uv;	//output 3D texture coordinate for the cubemap texture lookup
void main()
{ 	 	
	//clipspace position by multiplying the MVP matrix with the vertex position
	gl_Position = u_m4_model_view_proj*vec4(a_v3_position, 1.0);
	
	//output the object vertex vertex position as teh 3D texture coordinate
	uv = a_v3_position;
}