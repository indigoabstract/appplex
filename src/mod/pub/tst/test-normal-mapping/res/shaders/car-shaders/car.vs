uniform mat4 view_proj_matrix;
uniform mat4 inv_view_matrix;
//----------------------------------------------------------------------------------//
// Phenomenological car paint visual effect: Simulates build up of paint coats   //
// on the metallic surface of a car or any other object. This visual effect       //
// emulates the suspended layers of microflakes in the paint coat as well as       //
// blends between several colors of paint based on the viewing angle.         //
//                                          //
// Author: Natalya Tatarchuk                               //
//      (based on the original assembly shader written by John Isidoro)      //
//                                          //
// Used in ATI's Car demo for Radeon 9700 launch, found here:            //
//       http://www.ati.com/developer/demos/r9700.html               //
//                                          //
// (C) ATI Research, 2003                              //
//----------------------------------------------------------------------------------//

attribute vec4 rm_Vertex;
attribute vec2 rm_TexCoord0;
attribute vec3 rm_Normal;
attribute vec3 rm_Tangent;
attribute vec3 rm_Binormal;

varying vec2 Tex;
varying vec3 Tangent;
varying vec3 Binormal;
varying vec3 Normal;
varying vec3 View;
varying vec3 SparkleTex;

void main ()
{
   // Propagate transformed position out:
   gl_Position = view_proj_matrix * rm_Vertex;
   
   // Compute view vector: 
   View = vec3( normalize( (inv_view_matrix * vec4 (0.0, 0.0, 0.0, 1.0)) -  rm_Vertex ) );

   // Propagate texture coordinates:
   Tex = vec2( rm_TexCoord0.x, 1.0 - rm_TexCoord0.y );

   // Propagate tangent, binormal, and normal vectors to pixel shader:
   Normal   = rm_Normal;
   Tangent  = rm_Tangent;
   Binormal = rm_Binormal;
  
   // Compute microflake tiling factor:
   SparkleTex = vec3( rm_TexCoord0 * 20.0, 0.0 );   
}
