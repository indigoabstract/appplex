#ifdef GL_ES
precision highp float;
#endif

uniform float u_v1_time;
uniform vec2 u_v2_dim;
varying vec2 v_v2_tex_coord;
 
void main(void)
{
  // Be Cool
  vec2 p = v_v2_tex_coord - .5;
  p.x *= u_v2_dim.x / u_v2_dim.y;
  float a = atan(p.y, p.x);
  float r = length(p) + 0.0001;
 
  float b = 1.1 * sin(8.0 * r - u_v1_time - 2.0 * a);
  b = 0.3125 / r + cos(7.0 * a + b * b) / (100.0 * r);
  b *= smoothstep(0.0, 0.4, b);
 
  gl_FragColor = vec4(b * .3, .3 * b + 0.2 * sin(a + u_v1_time), b * 0.5, 1.0);
}
