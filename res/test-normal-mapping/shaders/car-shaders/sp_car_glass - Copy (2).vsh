uniform mat4 view_proj_matrix;

uniform vec4 view_position;

attribute vec4 rm_Vertex;
attribute vec3 rm_Normal;

varying vec3 vNormal;
varying vec3 vViewVec;

void main(void)
{
   vec3 Pos = rm_Vertex.xyz;

   vNormal    = rm_Normal;
   vViewVec   = view_position.xyz - Pos;
   vViewVec.z = vViewVec.z;
   
   gl_Position = view_proj_matrix * vec4(Pos,1.0);
}
