// https://www.shadertoy.com/view/4dsGW4
float xp = iResolution.x / 60.0;
float yp = iResolution.y / 30.0;

const vec4 l1 = vec4(0.1, 1.1, .9, 1.1);   // top horizontal
const vec4 l2 = vec4(0.0, 1.0, 0.0, 0.6);  // top left vertical
const vec4 l3 = vec4(1.0, 1.0, 1.0, 0.6);  // top right vertical
const vec4 l4 = vec4(0.1, 0.5, .9, 0.5);   // middle horizontal
const vec4 l5 = vec4(0.0, 0.4, 0.0, 0.0);  // lower left vertical
const vec4 l6 = vec4(1.0, 0.4, 1.0, 0.0);  // lower right vertical
const vec4 l7 = vec4(0.1, -0.1, .9, -0.1); // lower horizontal

float  notbetween(float x, float less, float more) {
    return 1.0 -  step(x, more) * step(less, x);
}


// distance between point p and line (l.x,l.y to l.z, l.w)
float ldist(in vec4 l,in vec2 p) { 
  vec2 lo = (l.zw - l.xy);
  float l2 = dot(lo, lo);
  float t = dot(p - l.xy, lo)/l2;
  
  if (l2 < 0.0 || t < 0.0001) {
    return length(p - l.xy);
  }
  if ( t > 1.0 ) {
    return length(p - l.zw);
  }
  return length(p - (l.xy + t * lo)); 
}

// use distance to l to return a brightness value 
float dist(in vec4 l,in vec2 p,in vec2 o,in vec2 s,in float w) {
    l = l * s.xyxy * 0.423;
    l += o.xyxy + vec4(0.28, 0.46, 0.28, 0.46) * s.x;
    return w * length(s)/(ldist(l, p)/w); // 14.9250;
}

// brightness of p for digit x, offset o, scale s, intensity w
float number(in vec2 p,in float x,in vec2 o,in vec2 s,in float w) {
    float not1or7, not4;
    return 
        notbetween(x, 0.5, 1.5) // not 1
          * notbetween(x, 3.5, 4.5) // and not 4
          * dist(l1, p, o, s, w)
      + notbetween(x, 0.5, 3.5) // not 1,2,3
          * notbetween(x, 6.5, 7.5) // and not 7
          * dist(l2, p, o, s, w)
      + notbetween(x, 4.5, 6.5) // not 4 or 5
          * dist(l3, p, o, s, w)
      + (not1or7 = (notbetween(x, 0.5, 1.5) * notbetween(x, 6.5, 7.5))) // not 1 or 7
          * step(0.5, x) // not  0
          * dist(l4, p, o, s, w)
      + mod(x + 1.0, 2.0) // not even
          * (not4 =  notbetween(x, 3.5, 4.5)) // not 4
          * dist(l5, p, o, s, w)
      + notbetween(x, 1.5, 2.5) // not 2
          * dist(l6, p, o, s, w)
      + not1or7 * not4 * dist(l7, p, o, s, w) // not 1, 7 or 4.
      - (0.2 + not1or7*0.23) * (0.2 + not1or7*0.23);
}

vec3 brighten(vec3 c) {
    return c / max(max(c.x, c.y), c.z);
}

void main() {
    vec2 s = vec2(1.0/(iResolution.x/xp), 1.0/(iResolution.y/yp));
    vec2 p = gl_FragCoord.xy/iResolution.xy;

    float t2 = fract(iGlobalTime /45.0) * 2.0 * 3.14159;
    vec2 cp = vec2(0.5, 0.5);
    float st2 = sin(t2);
    float ct2 = cos(t2);
    p += vec2(st2 * s.x/s.y, ct2) * (cp.x - p.x) * .4;
    p += (cp - p) * (st2 - 0.1);
    p.x -= st2/3.0;
    vec2 c = floor(p/s) * s;
    float cc = step(0.5, c.x);
    float t = floor(mod(fract(iGlobalTime/100.0) * (10.0 + cc * 90.0), 10.0));
      
    float n = number(
      c, t,
      vec2(0.018 + cc * .5, -0.14),
      vec2(.5, 1.85),
      0.165
    );

    float d = floor(mod(n,10.0));
    float b = number(p, d, c, s, 0.15);
    
    float sd = 1.0 - 1.0 / (1.0 + (d + 0.001)/9.0);
    vec3 col = brighten(vec3(b,b,b)) * 
      vec3(sin(sd * 3.14159),
          cos((d + 1.7) /sd) / 1.5 + 0.12,
      ((d * d + 19.0)/100.0) * sqrt(sd)
    );
    col *=  (1.0 - (1.0/(1.0 + b)));
    gl_FragColor = vec4(col, 1.0);
}
