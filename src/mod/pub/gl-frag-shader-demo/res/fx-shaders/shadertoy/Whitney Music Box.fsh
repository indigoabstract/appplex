// https://www.shadertoy.com/view/4dsSRM
// Whitney Music Box - Jim Bumgardner
// whitneymusicbox.org

const float rad = 0.9;
const float dots = 64.0;
const float duration = 180.0;
const vec3 colorsep = vec3(0,2.09,4.18);
const float PI = 3.1415926535897932384626433832795;
const float PI2 = 2.0*3.1415926535897932384626433832795;

void main(void) {
  vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
  float tm = mod(iGlobalTime,duration)/duration;
  p.y *= iResolution.y/iResolution.x;

  vec3 gradient = vec3(0.0);

  for (float i=1.0; i<=dots; i++)
  {
    float i2pi = i*PI2;
    float ang = mod(tm*i2pi, PI2);
    float amp = rad*(1.0-(i-1.0)/dots);
    float cang = i2pi/dots;
    float fade = 0.7 - pow(smoothstep(0.0,1.0,ang),2.0)*0.5;
    vec2 star_pos = vec2(cos(ang) * amp, -sin(ang) * amp);
    gradient += (cos(cang+colorsep) + 1.0/2.0) * ((fade / 384.0) / pow(length(star_pos - p), 1.5)) * fade;
  }
  gl_FragColor = vec4( gradient, 1.0);
}
