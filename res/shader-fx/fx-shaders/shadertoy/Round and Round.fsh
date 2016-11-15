// https://www.shadertoy.com/view/4dBGDt

// iChannel0: m2

float dist(vec2 a, vec2 b) {
  // return sqrt(((a.x-b.x)*(a.x-b.x) + .01/mod(u_time,2.) - (a.y-b.y)*(a.y-b.y)));
   return sqrt(((a.x-b.x)*(a.x-b.x) + .1/(mod(iGlobalTime,4.)*exp(mod(iGlobalTime,6.))) + (1.- 3.*abs(mod(iGlobalTime,3.) -1.))* (a.y-b.y)*(a.y-b.y)));
}
vec2 rotateVec(vec2 v, float a) { 
  //return vec2(v.x*cos(a)-v.y,v.y);
  return vec2(v.x*cos(a)-v.y*sin(a),v.x*sin(a)+v.y*cos(a));
}
vec2 rotateVec(vec2 v, float a, vec2 c) { 
  return vec2((v.x-c.x)*cos(a)-(v.y-c.y)*sin(a)+c.y,(v.x-c.x)*sin(a)+(v.y-c.y)*cos(a)+c.y);
}
void main(){
  vec2 c = vec2(.5,.5);
  vec2 u_texcoord = vec2(gl_FragCoord.x/iResolution.x, gl_FragCoord.y/iResolution.y);
  vec2 vtx = rotateVec(u_texcoord,mod(iGlobalTime,6.28),c);
  float dd = dist(vtx,c);
  float freq = texture2D(iChannel0, vec2(dd,1.)).x;
  vec3 color = vec3(freq > u_texcoord.y,1.4-.1*mod(iGlobalTime,18.),.1*mod(iGlobalTime,10.)) * ((2.*dd) / freq);
  gl_FragColor = vec4(color, 1.);
}
