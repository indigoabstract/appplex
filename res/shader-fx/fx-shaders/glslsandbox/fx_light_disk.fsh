// http://glsl.heroku.com/e#8024.0
#ifdef GL_ES
precision highp float;
#endif

uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

float makePoint(float x,float y,float fx,float fy,float sx,float sy,float t){
   float xx=x;
   float yy=y;
   return 0.1/(xx*xx+yy*yy);
}

void main( void ) {

   vec2 p=((v_v2_tex_coord*u_v2_dim)/u_v2_dim.x)*2.0-vec2(1.0,u_v2_dim.y/u_v2_dim.x);

   p=p*3.0;
   
   float x=p.x;
   float y=p.y;

   float t =0.0;

   float a=
       makePoint(x,y,3.5,2.9,0.3,0.3,t);
 

   vec3 d=vec3(a);
   
   gl_FragColor = vec4(d.x,d.y,d.z,1.0);
}
