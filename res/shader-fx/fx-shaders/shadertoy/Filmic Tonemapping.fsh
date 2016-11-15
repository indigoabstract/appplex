// https://www.shadertoy.com/view/MdfXWr
// with fake auto-exposure; hold/drag to compare bare/tonemapped (X axis); left: regular, right: filmic
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v)
  { 
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g;
  vec3 i1 = min( g.xyz, l.zxy );
  vec3 i2 = max( g.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289(i); 
  vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D.wyz - D.xzx;

  vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

  vec4 x_ = floor(j * ns.z);
  vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

  vec4 x = x_ *ns.x + ns.yyyy;
  vec4 y = y_ *ns.x + ns.yyyy;
  vec4 h = 1.0 - abs(x) - abs(y);

  vec4 b0 = vec4( x.xy, y.xy );
  vec4 b1 = vec4( x.zw, y.zw );

  //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
  //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
  vec4 s0 = floor(b0)*2.0 + 1.0;
  vec4 s1 = floor(b1)*2.0 + 1.0;
  vec4 sh = -step(h, vec4(0.0));

  vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
  vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1.xy,h.z);
  vec3 p3 = vec3(a1.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
  }


vec3 hue2rgb(float hue) {
    return clamp( 
        abs(mod(hue * 6.0 + vec3(0.0, 4.0, 2.0), 6.0) - 3.0) - 1.0, 
        0.0, 1.0);
}

vec3 hsv2rgbc(vec3 c) {
    vec3 rgb = hue2rgb(c.x);
    rgb = smoothstep(vec3(0.0),vec3(1.0),rgb);
    return c.z * mix(vec3(1.0), rgb, c.y);
}

vec3 lin2srgb(vec3 color) {
    vec3 S1 = sqrt(color);
    vec3 S2 = sqrt(S1);
    vec3 S3 = sqrt(S2);
    return 0.585122381 * S1 + 0.783140355 * S2 - 0.368262736 * S3;
}

vec3 ff_filmic_gamma3(vec3 linear) {
    vec3 x = max(vec3(0.0), linear-0.004);
    return (x*(x*6.2+0.5))/(x*(x*6.2+1.7)+0.06);
}

float specular(float r, float cos_Oh) {
    float a = r*r;
    float a2 = a*a;
    float Dd = max(0.00015, cos_Oh*cos_Oh*(a2 - 1.0)+1.0);
    return a2/(3.14159*Dd*Dd);
}

float specular2(float a, float cos_Ov, float cos_Ol) {
    float r = (a+1.0); 
    float k = (r*r) / 8.0;
    float ik = 1.0 - k;    
    float Gdlvh = 4.0 * (cos_Ov*ik + k) * (cos_Ol*ik + k);    
    return (1.0 / Gdlvh);
}

float fresnel(float cos_Od) {
    float p = (-5.55473*cos_Od-6.98316)*cos_Od;
    return 0.04 + (1.0 - 0.04)*pow(2.0, p);
}

float random(vec3 co) {
    return fract(sin(dot(co.xyz,vec3(12.9898,78.233,91.1743))) * 43758.5453);
}

float map(vec3 p) {
	float s = length(p)-1.0;
	
	return s + snoise((p*10.0 + iGlobalTime)-0.8)*0.005;
}

vec3 grad(vec3 p) {
	vec2 d = vec2(0.01, 0.0);
	return normalize(vec3(
			map(p + d.xyy) - map(p - d.xyy),
			map(p + d.yxy) - map(p - d.yxy),
			map(p + d.yyx) - map(p - d.yyx)
		));
}

float trace(vec3 rp, vec3 rd) {
	float r = 0.0;
	for (int i = 0; i < 8; ++i) {
		float d = map(rp + rd*r);
		if (abs(d) <= 0.001) break;
		r += d;
	}	
	return r;
}

float calcAO(vec3 p, vec3 n, float radius) {
    float s = radius/3.0;
    float ao = 0.0;
    for (int i = 1; i <= 3; ++i) {
        float dist = s * float(i);
    float t = map(p + n*dist);
        ao += max(0.0, (dist - t) / dist);
    }
    return 1.0 - (ao/3.0);
}

void main(void)
{
	float aspect = iResolution.x/iResolution.y;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv = uv*2.0-1.0;
	uv.x *= aspect;
	
	vec2 n2 = uv*1.2;
	// fake sphere normal
	vec3 n = vec3(n2, sqrt(1.0-n2.x*n2.x-n2.y*n2.y));
	// sphere uv
	vec2 suv = vec2(atan(n.x, n.z), n.y);
	vec3 p = n;
	
	float m = clamp((map(p)+0.005)*128.0, 0.0, 1.0);
	p -= n*trace(p, -n);
	n = grad(p);

	// material blend
	float ofsy = sin(suv.y*30.0+iGlobalTime)*0.5;
    float m2 = clamp((sin(suv.x*5.0+suv.y*4.0+iGlobalTime)-ofsy)*9.0+ofsy,0.0,1.0);
	
	// roughness
	float r = mix(0.1, 0.3, m2);
	
	vec3 albedo = hsv2rgbc(vec3((suv.y*0.5-suv.x*0.25+iGlobalTime*0.1)*0.2,mix(0.9,0.8,m),1.0));
	vec3 light = vec3(1.0,0.9,0.3) * 32.0;
    vec3 ambient = vec3(0.5,0.8,1.0) * 0.0625;

	
	// light vector
	vec3 l = normalize(vec3(cos(iGlobalTime),cos(iGlobalTime*0.1),sin(iGlobalTime))*10.0 - p);
	// camera vector
	vec3 v = normalize(vec3(0.0,0.0,5.0) - p);
	
	// half vector
	vec3 h = normalize(l + v);

	float cos_Ol = max(0.0, dot(n, l));
	float cos_Ov = max(0.0, dot(n, v));
    float cos_Oh = max(0.0, dot(n, h));
	float cos_Od = max(0.0, dot(v,h));
	
	float light_lum = cos_Ol / 3.14159;
	float spec_lum = specular(r, cos_Oh) * specular2(r,cos_Ov,cos_Ol) * fresnel(cos_Od);
	float amb_lum = max(0.0, dot(n, vec3(0.0,0.0,1.0)));
	
	vec3 col = albedo * light_lum + vec3(spec_lum) * cos_Ol;
	col *= light;
	
	// ambient occlusion
	float ao = calcAO(p,n,0.1);
	// ambient light
	col += albedo * ambient * ao;

    col *= amb_lum;
	
	vec2 mouse = clamp(iMouse.xy / iResolution.xy, 0.0, 1.0);

    // add sensor noise
	float tt = mod(iGlobalTime, 5.0);
	vec3 ns = vec3(
		random(vec3(gl_FragCoord.xy*0.001,tt)),
		random(vec3(gl_FragCoord.xy*0.01,tt)),
		random(vec3(gl_FragCoord.xy*0.1,tt)));
	col = mix(col, vec3(1.0), ns*0.01);
	
	float lightness = dot(l, vec3(0.0,0.0,-1.0))*0.5+0.5;
	
	// adjust exposure
	col *= pow(2.0, mix(-4.0, 4.0, lightness));

	mouse = mouse*2.0-1.0;
	mouse.x *= aspect;
	
	if (uv.x > mouse.x)
		col = clamp(ff_filmic_gamma3(col),0.0,1.0);	
	else
		col = lin2srgb(clamp(col,0.0,1.0));	
	
	col *= clamp(abs(uv.x-mouse.x)*320.0, 0.0, 1.0);
	
	gl_FragColor = vec4(col,1.0);
}
