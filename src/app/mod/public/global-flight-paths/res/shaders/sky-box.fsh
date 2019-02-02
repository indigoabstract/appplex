//@es #version 300 es
//@dt #version 330 core

#ifdef GL_ES
	precision highp float;
#endif

layout(location=0) out vec4 vFragColor;	//fragment shader output

uniform samplerCube u_scm_skybox;	//cubemap texture sampler

//input from the vertex shader
smooth in vec3 uv;	//interpolated 3D texture coordinate

void main()
{
	//return the colour from the cubemap texture 
	vFragColor = texture(u_scm_skybox, uv);
}