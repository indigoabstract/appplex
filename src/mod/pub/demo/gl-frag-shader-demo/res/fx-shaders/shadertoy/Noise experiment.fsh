// https://www.shadertoy.com/view/MsXGD8
/** Uses fbm / noise aparatus from iq's clouds on shadertoy, **/
/** some digits were changed to protect the innocent **/

mat3 m = mat3( 0.0,  0.8,  0.60,
              -0.80,  0.36, -0.48,
              -0.60, -0.48,  0.64 );


float cubicPulse( float c, float w, float x )
{
    x = abs(x - c);
    if(x > w) { return 0.0; }
    x /= w;
    return 1.0 - x*x*(3.0-2.0*x);
}

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}

float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);
    float n = p.x + p.y*57.0 + 113.0*p.z;

    float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                        mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
                    mix(mix( hash(n+113.0), hash(n+114.0),f.x),
                        mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
    return 1.0 - sqrt(res);
}

float fbm( vec3 p )
{
    float f;    
    f  = 0.6000*noise( p ); p = m*p*2.02;
    f += 0.200*noise( p ); p = m*p*2.0130;
    f += 0.06250*noise( p ); p = m*p*2.0370;
    f += 0.0275*noise( p ); p = m*p*2.02;
    f += 0.00513*noise(p * 2.2);
    return f;
}

float iat(in vec2 q) {
  return fbm(vec3(q + 0.3 * vec2(iGlobalTime, iGlobalTime), 0.626));
}

void main(void) {
  vec2 q = gl_FragCoord.xy / iResolution.x;

  vec2 n = q + vec2(0.0, 0.001);
  vec2 e = q + vec2(0.001, 0.0);
  vec2 s = q + vec2(0.0, -0.001);
  vec2 w = q + vec2(-0.001, 0.0);

  float ifac = 15.0;
  float i  = iat(q * 20.0  );
  float ni = iat(n * (20.0 - ifac + ifac  * i));
  float ei = iat(e * 20.0);
  float si = iat(s * 20.0);
  float wi = iat(w * 20.0);
  
  vec3 no = (normalize(vec3( (ni - si), (ei - wi), sqrt((ei - wi)*(ei - wi) + (ni - si)*(ni - si)))));
  float dif = 1.5 * dot( no, normalize(vec3(-0.13, 0.4, 0.167)));
  vec3 col = vec3(1.8 * cubicPulse(0.6, 0.35, i), 1.1 * cubicPulse(0.45,0.35,i), 0.3 * cubicPulse(0.2,0.3,i)) / 1.0;
  col *= dif;
  gl_FragColor = vec4(col, 1.0);
}
