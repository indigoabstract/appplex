
//---------------------------------------------------------
// Shader:   GoldenDunes2.glsl         2014 by Passion 
// https://www.shadertoy.com/view/Xls3R4
// now with stripes    http://glslsandbox.com/e#30646
// optimized           http://glslsandbox.com/e#30649
//---------------------------------------------------------

#ifdef GL_ES
  precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

//---------------------------------------------------------
// 2d noise functions from https://www.shadertoy.com/view/XslGRr
vec2 hash( vec2 p )                       // rand in [-1,1]
{
  p = vec2( dot(p,vec2(127.1,311.7)),
            dot(p,vec2(269.5,183.3)) );
  return -1. + 2.*fract(sin(p+20.)*53758.5453123);
}

float noise( in vec2 p ) 
{
  vec2 i = floor((p)), f = fract((p));
  vec2 u = f*f*(3. - f - f);
  return mix( mix( dot( hash( i + vec2(0.,0.) ), f - vec2(0.,0.) ), 
                   dot( hash( i + vec2(1.,0.) ), f - vec2(1.,0.) ), u.x),
              mix( dot( hash( i + vec2(0.,1.) ), f - vec2(0.,1.) ), 
                   dot( hash( i + vec2(1.,1.) ), f - vec2(1.,1.) ), u.x), u.y);
}
//---------------------------------------------------------
#define NB 10       // number or gabor blobs
#define SIZE 0.22   // size of gabor blobs
                    // freq tuned by mouse.x

float rnd(int i, int j)
{
  return noise(vec2(1.5+float(i), 1.5+float(j)));
}

// see https://www.shadertoy.com/view/MsG3Wm
float DuneStripes (vec2 uv, float d, float freq, float time)
{
  float hv = 0.;
  for (int i=0; i<NB; i++) 
  {
    vec2 pos = vec2(rnd(i,0), rnd(i,1));
    vec2 dir = (12.+d)*vec2(rnd(i,2),rnd(i,3)) - d;
    hv += 0.4*sin(dot(uv-pos, freq*dir) * 6. + time);
  }
  return hv;
}
//---------------------------------------------------------
float distanceFunction(vec3 p)
{
  vec4 n = vec4(0.0,1.0,0.0,1.0); 
//  float disp = .3*sin(.4*p.z)*sin(p.x+noise(p.zx))*2.5;
  float disp = .3*sin(1.4*p.x+cos(p.z*1.3))-(1.0-abs(sin(p.x+noise(p.zx))))*.4;
//	    + 0.0002*sin(noise(p.xy)+(p.z+sin(p.y))*144.);

  float plane = dot(p,n.xyz) + n.w;
  return (plane+disp);
}
 
vec3 getNormal(vec3 p)
{
  const float d = 0.0001;
  float dv = distanceFunction(p);
  return normalize (vec3 (distanceFunction(p+vec3(d,0.0,0.0))-dv,
                          distanceFunction(p+vec3(0.0,d,0.0))-dv,
                          distanceFunction(p+vec3(0.0,0.0,d))-dv));
}
 
//---------------------------------------------------------
void main() 
{
  float time2 = time*0.1;
  vec2 pos = (gl_FragCoord.xy*2.0 -resolution) / resolution.y;
  vec3 bg = mix(vec3(0.0,0.0,0.9*pos.y),vec3(1.5),.3);  

  vec3 camPos = vec3(sin(time2*0.5)*2.0, -0.1, -time2);
  vec3 camDir = vec3(.3, -0.2, -1.0);
  vec3 camUp = vec3(0.0, 1.0, 0.0);
  vec3 camSide = cross(camDir, camUp);
  float focus = 1.8;
  
  vec3 col = vec3(0.0);
  vec3 rayDir = normalize(camSide*pos.x + camUp*pos.y + camDir*focus);
  
  float t = 0.0, d;
  vec3 posOnRay = camPos;
 
  for(int i=0; i<64; ++i)
  {
    d = distanceFunction(posOnRay);
    t += d;
    posOnRay = camPos + t*rayDir;
  }

  // add wave texture
  vec3 tex = vec3(0.2*sin(noise(posOnRay.xy)+(posOnRay.y+sin(posOnRay.y))*244.) );

  vec3 l = vec3 (sin(time), 111., -111.0);
  vec3 normal = getNormal(posOnRay);
  
  if(abs(d) < 0.5)
  {
    float diff = dot(normal,normalize(l));
    float spe = 1.5*pow(clamp(dot(reflect(rayDir, normal), l), 0., 1.), 150.);
    vec3 brdf = 1.5*vec3(.10, .11, .11);
    float fre = .2*pow(clamp(1. + dot(normal, rayDir), 0., 1.), 2.);
    brdf += 1.30*diff*vec3(1., .9, .75);
    col = diff*vec3(.8,.6,.3)*spe*brdf + fre*tex;
    gl_FragColor = vec4(mix(col,bg,smoothstep(.1,.2,t*.015)),1.0);
  }
  else gl_FragColor = vec4(bg,1.0);
}
