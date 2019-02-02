// http://www.ati.com/developer/demos/r9700.html

uniform mat4 u_m4_model;
uniform mat4 u_m4_model_view_proj;
uniform mat4 u_m4_view;
uniform mat4 u_m4_view_inv;
uniform mat4 u_m4_projection;

attribute vec3 a_v3_position;
attribute vec2 a_v2_tex_coord;
attribute vec3 a_v3_normal;
attribute vec3 a_v3_tangent;
attribute vec3 a_v3_bitangent;

varying vec3 v_v3_pos_ws;
varying vec3 v_v3_normal_ws;
varying vec2 Tex;
varying vec3 Tangent;
varying vec3 Binormal;
varying vec3 Normal;
varying vec3 View;
varying vec2 SparkleTex;

void main()
{
	v_v3_pos_ws = (u_m4_model * vec4(a_v3_position, 1.0)).xyz;
	v_v3_normal_ws = normalize((u_m4_model * vec4(a_v3_normal, 1.0)).xyz);
   // Propagate transformed position out:
   gl_Position = u_m4_model_view_proj * vec4(a_v3_position, 1.0);
   //gl_Position = u_m4_projection * u_m4_view * vec4(a_v3_position, 1.0);
   
   // Compute view vector: 
   View = vec3( normalize( (u_m4_view_inv * vec4 (0.0, 0.0, 0.0, 1.0)) -  vec4(a_v3_position, 1.0) ) );

   // Propagate texture coordinates:
   Tex = vec2( a_v2_tex_coord.x, 1.0 - a_v2_tex_coord.y );

   // Propagate tangent, binormal, and normal vectors to pixel shader:
   Normal   = a_v3_normal;
   Tangent  = a_v3_tangent;
   Binormal = a_v3_bitangent;
  
   // Compute microflake tiling factor:
   SparkleTex = a_v2_tex_coord * 20.0;   
}
