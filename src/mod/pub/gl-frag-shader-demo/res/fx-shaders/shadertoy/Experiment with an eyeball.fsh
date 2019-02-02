// https://www.shadertoy.com/view/XslXW8

// iChannel0: c3
// iChannel1: c2

mat2 m = mat2( 0.80,  0.60, -0.60,  0.80 );

float hash( float n )
{
    return fract(sin(n)*43758.5453);
}


float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0;

    float res = mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
                    mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y);
    return res;
}

float fbm( vec2 p )
{
    float f = 0.0;

    f += 0.50000*noise( p ); p = m*p*2.02;
    f += 0.25000*noise( p ); p = m*p*2.03;
    f += 0.12500*noise( p ); p = m*p*2.01;
    f += 0.06250*noise( p ); p = m*p*2.04;
    f += 0.03125*noise( p );

    return f/0.984375;
}

vec3 ExtractCameraPos(mat4 a_modelView)
{
  mat3 rotMat =mat3(a_modelView[0].xyz,a_modelView[1].xyz,a_modelView[2].xyz);
  vec3 d =  a_modelView[3].xyz;
 
  vec3 retVec = -d * rotMat;
  return retVec;
}

float smoothCurves(float x)
{
  return (x<0.)?0.:x;
}

float f(vec3 c)
{
    float d1 = length(c)-1.3;
    float d2 = distance(c,vec3(0.,0.,1.))-.5;
    float e = mix(d1,d2,smoothCurves(d1-d2));
    return e;
}

vec3 normal(vec3 p) {
  vec3 e = vec3(.1, 0.0, 0.0);
  vec3 n;
  n.x = f(p + e.xyy) - f(p - e.xyy);
  n.y = f(p + e.yxy) - f(p - e.yxy);
	n.z = f(p + e.yyx) - f(p - e.yyx);
	return normalize(n);
}

vec3 cool(vec2 p)
{
  vec2 q = vec2(p.y * cos(p.x), p.y*sin(p.x));
  float f = fbm (q);
  
	
	vec3 pupil = vec3(0.);
	vec3 border = vec3(0.9,0.6,0.2);
	vec3 blue= mix( vec3( 0.0, 0.3, 0.4 ), vec3(0.2,0.5,0.4), fbm (20.*q));

	vec3 nerf = mix (vec3( .8,0.,0.), vec3(1.),abs(cos(p.x*2. + f*5.)) +.4 );	
		
	f = smoothstep( 0.3, 1.0, fbm( vec2(25.0*p.x,6.0*p.y) ) );
	border = mix( border, vec3(1.), f );
	blue = mix( blue, vec3(1.), f );
	
	f = smoothstep( 0.3, 1.0, fbm( vec2(50.0*p.x,12.*p.y) ) );
	border = mix( border, vec3(0.), f );
	blue = mix( blue, vec3(0.), f );
	
	vec3 color = mix(pupil , border, smoothstep(.07, .1, p.y/3.14));
	color = mix(color, blue, smoothstep(.06, .15, p.y/3.14));
  color = mix(color, vec3(0.), smoothstep(.15, .16, p.y/3.14));
  color = mix(color, vec3(1.), smoothstep(.15, .17, p.y/3.14));  
	color = mix(color, nerf, smoothstep(.17, .5, p.y/3.14));
	
	return color;
	
}

vec2 cart2spherical( vec3 pos )
{
        // code from colbert et al.

        vec3 npos = normalize(pos);

        float phi = atan(npos.y,npos.x);
        phi = (phi>0.)?phi:(2. * 3.14 + phi);

        float theta = acos(npos.z);

        return vec2( phi , theta );
}

void main()
{
  vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy.xy;
  p.x *= iResolution.xy.x/iResolution.xy.y;

  vec3 l = normalize(vec3(-1.,1.,1.));
    // camera
    vec2 pos = (iMouse.xy / iResolution.xy) * vec2(6.28, 3.14) - vec2(3.14, 1.57); // rotation en x sur 2pi et sur pi en y
    float cosx = cos(pos.x);
    float sinx = sin(pos.x);
    float cosy = cos(pos.y);
    float siny = sin(pos.y);
    vec3 ro = vec3(sinx * cosy, -siny, cosx * cosy) * 3.5;
    vec3 ww = normalize(vec3(0.0) - ro);
    vec3 uu = normalize(cross( vec3(0.0,1.0,0.0), ww ));
    vec3 vv = normalize(cross(ww,uu));
    vec3 rd = normalize( p.x*uu + p.y*vv + 1.5*ww );

  vec3 color = vec3(.5,.3,.2)*length(p);
  color = pow(textureCube(iChannel1, rd).rgb, vec3(2.2));
  float e, d =0.;

  vec3 c;
  for(int i = 0; i<64; i++)
  {
    c = ro + d *rd;

    e = f(c);
    if(e < .001)
    {
       break;
    }
    d+=e;
  }

  // Refraction and reflection
  if(e < .001)
  {
    vec3 n = normal(c);
    vec3 newd = refract(rd,n,0.75);
    float d2 = 0.;
	vec3 m ;
    for(int j =0; j<32; j++)
    {
      m = c + d2 * newd  ;
      float e2 = length(m)-1.;
      if(e2<.005)
      {
        color = cool(cart2spherical(m))*(1.-d2)*1.2;
        break;
      }
      d2+=e2;
    }
    
      // Add the reflection
      vec3 s = normalize(l);
      vec3 v = -rd;
      vec3 h = normalize(v+s);
      color += pow(max(0.0, dot(n,h)), 125.);
      color += pow( 0.4*textureCube(iChannel0, reflect(rd, n)).rgb, vec3(4.4));
	  
	  float opacity = length(c-m);
	  
	  color *= ( 1.75*opacity);
  }
  
  gl_FragColor = vec4(color, 1.0); 
}
