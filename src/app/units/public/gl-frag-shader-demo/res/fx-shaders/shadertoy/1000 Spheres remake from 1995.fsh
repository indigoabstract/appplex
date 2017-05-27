// https://www.shadertoy.com/view/lds3z8

// iChannel0: t8
// iChannel1: t3

// 1000 spheres in a 10x10x10 cube pattern, ray tracing with grid marching
//
// Version 1.2 (2013-03-12)
// Simon Stelling-de San Antonio
//
// Remake of my raytracing image from March 1995,
// see http://home.wtal.de/ss/html/pix/1000kug.jpg
// and http://home.wtal.de/ss/html/pix/1000zoom.jpg
//
// Many thanks to Iñigo Quílez (iq) for articles, example source codes and Shadertoy.
//
// V1.0  2013-03-12  Initial public version with ray marching
// V1.1  2013-03-13  Bugfix in ray reflection
// V1.2  2013-03-13  Replaced ray marching by "grid marching" and exact ray/sphere intersection

bool isInCube(vec3 pos)
{
  return max(max(abs(pos.x),abs(pos.y)),abs(pos.z)) < 5.0;
}

float tToPlane(vec3 o, vec3 d)
{
    float t = 9999999.9;
    if (abs(d.x) > 0.0001)
    {
      float tt = -(5.0 * sign(d.x) + o.x)/d.x;
      if (tt >= 0.0)
      {
        vec2 s = o.yz + tt * d.yz;
        if ((abs(s.x) <= 5.0) && (abs(s.y) <= 5.0)) {
          t = tt;
        }
      }
    }
    return t;
}

float tToCube(vec3 ro, vec3 rd)
{
  float t = 0.0;
  if (!isInCube(ro)) {
    t = min(tToPlane(ro.xyz, rd.xyz),
        min(tToPlane(ro.yxz, rd.yxz),
            tToPlane(ro.zxy, rd.zxy)
        ));
    if (t == 9999999.9) t = 0.0;
  }
  return t;
}

vec4 intersectSphere(vec3 pos, vec3 v, vec3 m) // Ray position and direction, Sphere center
{
  vec3 a = pos - m;
  float ph = dot(a,v);
  float d = ph*ph - dot(a,a) + 0.0625;
  vec4 res = vec4(-1.0);
  if (d >= 0.0) {
    res = vec4(pos - (ph + sqrt(d))*v, 0.0);
  }
  return res;
}

void main(void)
{
  vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
  p.x *= (iResolution.x / iResolution.y);

  // camera
  float camtime = 0.27*iGlobalTime;
  vec3 ro = (3.9+1.5*cos(0.61*camtime))*vec3(2.5*sin(0.25*camtime),1.02+1.01*cos(0.13*camtime),2.51*cos(0.25*camtime));
  vec3 ww = normalize(vec3(0.0) - ro);
  vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
  vec3 vv = normalize(cross(ww,uu));
  vec3 rd = normalize( p.x*uu + p.y*vv + 2.5*ww );

  vec3 last_test_box = vec3(8.0);
  float sh = 1.0;

  ro += tToCube(ro,rd)*rd;

  for (int j = 0; j < 20; j++) {
    vec3 test_box = floor(ro);
    if ((test_box != last_test_box) && isInCube(ro)) {
      last_test_box = test_box;
      vec3 m = test_box + 0.5;
      vec4 hit = intersectSphere(ro,rd,m);
      if (hit.w == 0.0) {
        vec3 n = normalize( hit.xyz - m );
        rd -= 2.0 * dot(n,rd) * n;
        ro = hit.xyz + 0.25 * rd;
        sh *= 0.89;
      }
    }
    ro = ro + 0.91 * rd;
  }

  float li = 0.0;
  float ang = abs(rd.y);
  vec3 col1;
  vec3 col2;
  vec2 ip = ro.xz + ((33.3 - ro.y) / rd.y) * rd.xz;
  if (rd.y < 0.0) {
    // earth color
    vec2 fc = fract(0.03*ip);
    float mix1 = texture2D(iChannel0,fc).x;
    col1 = mix( vec3(0.99, 0.78, 0.39), vec3(0.71, 0.58, 0.19), mix1);
    col2 = mix( vec3(0.75, 0.55, 0.16), vec3(0.65, 0.25, 0.16), mix1);
  } else {
    // sky color
    vec2 fc = fract(0.001*ip + 0.5);
    float mix1 = 1.0 - texture2D(iChannel1,fc).x;
    mix1 = clamp( (mix1 * 1.7 - 0.5), 0.0, 1.0);
    ang = clamp(ang * 1.5,0.0,1.0);
    col1 = mix( vec3(0.39, 0.78, 1.0), vec3(0.19, 0.58, 0.7), mix1);
    col2 = mix( vec3(0.90, 0.90, 0.9), vec3(0.03, 0.24, 0.5), mix1);
    // light
    vec3 light = normalize(vec3(1.0,0.9,0.3));
    li = pow( max(dot(rd,light),0.0), 88.1);
  }
  vec3 col = mix(col1,col2,ang);
  col = mix(col, vec3(1.0), li); // apply light
  col = mix(vec3(0.3), col, sh); // apply shadow
  gl_FragColor = vec4(col,1.0);
}
