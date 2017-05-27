// https://www.shadertoy.com/view/4ssSRM
// Log Spiral - Jim Bumgardner

const float PI = 3.1415926535897932384626433832795;
const float PId2 = PI/2.0;

void main(){

  vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
  p.y *= iResolution.y/iResolution.x;

  // controls arms for each channel (2 layers)
  vec3 n1_3 = vec3(-6.0,5.0,3.0);
  vec3 m1_3 = vec3(3.0,11.0,-12.0);

  // controls zoom effect for each layer
  vec3 n4_6 = 10.0*iGlobalTime*vec3(.1,.11,.111);
  vec3 m4_6 = 10.0*iGlobalTime*vec3(.2,.22,.222);

  // color width for each channel
  vec3 n7_9 = vec3(0.5);
  vec3 m7_9 = vec3(0.5);

  // color center for each channel
  vec3 n10_12 = vec3(0.5);
  vec3 m10_12 = vec3(0.5);

  // Layer mix
  float mixv = cos(iGlobalTime*.1)*0.5+0.5;

  float a = atan(p.x, p.y);
  float d = log(length(p));
  // two layer version...
  gl_FragColor = vec4(mix(sin(d * n1_3 + vec3(a,-a-PId2,a+PId2) - n4_6) * n7_9 + n10_12, 
                          sin(d * m1_3 + vec3(a,-a-PId2,a+PId2) - m4_6)*sin(a*6.0) * m7_9 + m10_12, 
                          mixv), 
                      1.0);
}
