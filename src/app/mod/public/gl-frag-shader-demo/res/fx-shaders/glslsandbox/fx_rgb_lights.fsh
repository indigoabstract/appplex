// http://glsl.heroku.com/e#6158.0
// By @paulofalcao
//
// Blobs

#ifdef GL_ES
precision highp float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;

float makePoint(float x,float y,float fx,float fy,float sx,float sy,float t){
   float xx=x+tan(t*fx)*sx;
   float yy=y+tan(t*fy)*sy;
   return 1.0/sqrt(xx*xx+yy*yy);
}

void main( void ) {

   vec2 p=((v_v2_tex_coord*u_v2_dim)/u_v2_dim.x)*2.0-vec2(1.0,u_v2_dim.y/u_v2_dim.x);

   p=p*2.0;
   
   float x=p.x;
   float y=p.y;

   float a=
       makePoint(x,y,3.3,2.9,0.3,0.1,u_v1_time);
   a=a+makePoint(x,y,1.9,2.0,0.4,0.4,u_v1_time);
   a=a+makePoint(x,y,0.8,0.7,0.4,0.5,u_v1_time);
   a=a+makePoint(x,y,2.3,0.1,0.6,0.3,u_v1_time);
   a=a+makePoint(x,y,0.8,1.7,0.5,0.4,u_v1_time);
   a=a+makePoint(x,y,0.3,1.0,0.4,0.4,u_v1_time);
   a=a+makePoint(x,y,1.4,1.7,0.4,0.5,u_v1_time);
   a=a+makePoint(x,y,1.3,2.1,0.6,0.3,u_v1_time);
   a=a+makePoint(x,y,1.8,1.7,0.5,0.4,u_v1_time);   
   
   float b=
       makePoint(x,y,1.2,1.9,0.3,0.3,u_v1_time);
   b=b+makePoint(x,y,0.7,2.7,0.4,0.4,u_v1_time);
   b=b+makePoint(x,y,1.4,0.6,0.4,0.5,u_v1_time);
   b=b+makePoint(x,y,2.6,0.4,0.6,0.3,u_v1_time);
   b=b+makePoint(x,y,0.7,1.4,0.5,0.4,u_v1_time);
   b=b+makePoint(x,y,0.7,1.7,0.4,0.4,u_v1_time);
   b=b+makePoint(x,y,0.8,0.5,0.4,0.5,u_v1_time);
   b=b+makePoint(x,y,1.4,0.9,0.6,0.3,u_v1_time);
   b=b+makePoint(x,y,0.7,1.3,0.5,0.4,u_v1_time);

   float c=
       makePoint(x,y,3.7,0.3,0.3,0.3,u_v1_time);
   c=c+makePoint(x,y,1.9,1.3,0.4,0.4,u_v1_time);
   c=c+makePoint(x,y,0.8,0.9,0.4,0.5,u_v1_time);
   c=c+makePoint(x,y,1.2,1.7,0.6,0.3,u_v1_time);
   c=c+makePoint(x,y,0.3,0.6,0.5,0.4,u_v1_time);
   c=c+makePoint(x,y,0.3,0.3,0.4,0.4,u_v1_time);
   c=c+makePoint(x,y,1.4,0.8,0.4,0.5,u_v1_time);
   c=c+makePoint(x,y,0.2,0.6,0.6,0.3,u_v1_time);
   c=c+makePoint(x,y,1.3,0.5,0.5,0.4,u_v1_time);
   
   vec3 d=vec3(a,b,c)/32.0;
   
   gl_FragColor = vec4(d.x,d.y,d.z,1.0);
}
