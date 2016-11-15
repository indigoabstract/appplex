// https://www.shadertoy.com/view/Xdj3zh
float time = iGlobalTime;
vec3 resolution = iResolution;
vec4 mouse = iMouse;
struct camera {
  vec3 pos;
  vec3 facing;
};

struct hitInfo {
    vec3 lv;
    vec3 cv;
    vec3 normal;
    vec3 pos;
};

struct lightInfo {
   vec3 pos;
   vec3 dColor;
   vec3 sColor;
};

lightInfo light1, light2;
camera cam;

vec3 bg(vec2 p) {
 //p.x += floor(mod(time, .2)*10.);
 if (floor(mod(floor(p.x)+floor(p.y), 2.0)) <1.)
    return vec3(0.);
   return vec3(1.);
}

vec3 getBG(vec2 p) {
  p.x += cos(.5*time); 
  p.y += sin(.5*time);
  p += vec2(0.5*sin(p.y)*sin(time),
          -0.5*sin(p.x)*sin(time));
  p*=4.;
//  p.x = p.x + p.x*cos(time);
 // p.y = p.y + p.y*sin(time);
  return bg(p);
}

vec3 getColor(hitInfo hit, lightInfo lite) {
    vec3 col;
    
    if (dot( vec3(0.,0., -1.), hit.normal)==0.) {col=vec3(0.);}
    else {
       col = 0.7*getBG(
               vec2( hit.pos.xy +
                 hit.normal.xy*dot(vec3(0.,0., 1.)-hit.pos, vec3(0.,0.,1.))/dot(hit.normal, vec3(0.,0.,1.))
               )
             ).xxx;
    }

    float i = clamp(dot(hit.lv, hit.normal), 0., 1.);
    col += i*lite.dColor;

    i = pow(clamp(dot(hit.cv, reflect(hit.lv, hit.normal)), 0., 1.), 4.0);
    col += i *lite.sColor;
    return col;
}

const int MAX_STEPS = 100;

float raymarch(camera o, vec3 j, float d) {

   vec3 r = o.pos;
   vec3 s = o.facing*0.1;
   float n, dist = n = distance(o.pos, j);
   for (int i = 0; i < MAX_STEPS; i++) {
     if (n <= d) {
        return length(r);
     }
     r += s;
     dist = n;
     n = distance(r, j);
   }
   return 0.;
}

void main()
{
    vec2 p = -2.0 + 4.0 * gl_FragCoord.xy / resolution.xy;
    p.x *= resolution.x/resolution.y;

    cam.pos  = vec3(p, 0.);
    cam.facing = vec3(0.,0.,1.);

    vec3 r = cam.pos;
    vec4 col = vec4(getBG(cam.pos.xy), 0.5);
    
    vec3 cc = vec3(cos(time), sin(time), 0.);

    lightInfo light1 = lightInfo(vec3(0.,0., 1.+cos(time)*0.8), vec3(1.,1.,1.), vec3(1.,1., 1.)
    );
    lightInfo light2 = lightInfo(vec3(cos(time*8.),0.5,2.), vec3(0.,0., 1.), vec3(0.,1., 1.)
    );

    float a = acos(sign(cam.pos.y-cc.y)*dot(normalize(cam.pos.xy-cc.xy), vec2(1., 0.)));
    float sa = 0.5+0.25*cos(10.*a+time)*cos(20.*a);

    float d = raymarch(cam, cc, 0.5);

    if (d > 0.) {

        d =length(cam.pos- cc)/0.5;
        r = vec3(cam.pos.xy, sqrt(1.-d*d));

       col = vec4(getColor(
          hitInfo(normalize(light1.pos-r), normalize(cam.pos-r), normalize(r-cc), r), light1
       ), 1.);
     //  col += vec4(getColor(
     //     hitInfo(normalize(light2.pos-r), normalize(cam.pos-r), normalize(r-cc), r)
      // ), 1.);
    }

    gl_FragColor = col;

}
