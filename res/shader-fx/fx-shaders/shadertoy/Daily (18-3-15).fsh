// https://www.shadertoy.com/view/4t2Gzz
#define GLSLIFY 1

float orenNayarDiffuse_3_0(
  vec3 lightDirection,
  vec3 viewDirection,
  vec3 surfaceNormal,
  float roughness,
  float albedo) {
  
  float LdotV = dot(lightDirection, viewDirection);
  float NdotL = dot(lightDirection, surfaceNormal);
  float NdotV = dot(surfaceNormal, viewDirection);

  float s = LdotV - NdotL * NdotV;
  float t = mix(1.0, max(NdotL, NdotV), step(0.0, s));

  float sigma2 = roughness * roughness;
  float A = 1.0 + sigma2 * (albedo / (sigma2 + 0.13) + 0.5 / (sigma2 + 0.33));
  float B = 0.45 * sigma2 / (sigma2 + 0.09);

  return albedo * max(0.0, NdotL) * (A + B * s / t) / 3.14159265;
}


//
// Description : Array and textureless GLSL 2D/3D/4D simplex
//               noise functions.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//

vec3 mod289_1_1(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289_1_1(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute_1_2(vec4 x) {
     return mod289_1_1(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt_1_3(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise_1_4(vec3 v)
  {
  const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
  const vec4  D_1_5 = vec4(0.0, 0.5, 1.0, 2.0);

// First corner
  vec3 i  = floor(v + dot(v, C.yyy) );
  vec3 x0 =   v - i + dot(i, C.xxx) ;

// Other corners
  vec3 g_1_6 = step(x0.yzx, x0.xyz);
  vec3 l = 1.0 - g_1_6;
  vec3 i1 = min( g_1_6.xyz, l.zxy );
  vec3 i2 = max( g_1_6.xyz, l.zxy );

  //   x0 = x0 - 0.0 + 0.0 * C.xxx;
  //   x1 = x0 - i1  + 1.0 * C.xxx;
  //   x2 = x0 - i2  + 2.0 * C.xxx;
  //   x3 = x0 - 1.0 + 3.0 * C.xxx;
  vec3 x1 = x0 - i1 + C.xxx;
  vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
  vec3 x3 = x0 - D_1_5.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

// Permutations
  i = mod289_1_1(i);
  vec4 p = permute_1_2( permute_1_2( permute_1_2(
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 ))
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

// Gradients: 7x7 points over a square, mapped onto an octahedron.
// The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
  float n_ = 0.142857142857; // 1.0/7.0
  vec3  ns = n_ * D_1_5.wyz - D_1_5.xzx;

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
  vec4 a1_1_7 = b1.xzyw + s1.xzyw*sh.zzww ;

  vec3 p0_1_8 = vec3(a0.xy,h.x);
  vec3 p1 = vec3(a0.zw,h.y);
  vec3 p2 = vec3(a1_1_7.xy,h.z);
  vec3 p3 = vec3(a1_1_7.zw,h.w);

//Normalise gradients
  vec4 norm = taylorInvSqrt_1_3(vec4(dot(p0_1_8,p0_1_8), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
  p0_1_8 *= norm.x;
  p1 *= norm.y;
  p2 *= norm.z;
  p3 *= norm.w;

// Mix final noise value
  vec4 m = max(0.6 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
  m = m * m;
  return 42.0 * dot( m*m, vec4( dot(p0_1_8,x0), dot(p1,x1),
                                dot(p2,x2), dot(p3,x3) ) );
  }



//
// Description : Array and textureless GLSL 2D simplex noise function.
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//

vec3 mod289_2_9(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289_2_9(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute_2_10(vec3 x) {
  return mod289_2_9(((x*34.0)+1.0)*x);
}

float snoise_2_11(vec2 v)
  {
  const vec4 C = vec4(0.211324865405187,  // (3.0-sqrt(3.0))/6.0
                      0.366025403784439,  // 0.5*(sqrt(3.0)-1.0)
                     -0.577350269189626,  // -1.0 + 2.0 * C.x
                      0.024390243902439); // 1.0 / 41.0
// First corner
  vec2 i  = floor(v + dot(v, C.yy) );
  vec2 x0 = v -   i + dot(i, C.xx);

// Other corners
  vec2 i1;
  //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
  //i1.y = 1.0 - i1.x;
  i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
  // x0 = x0 - 0.0 + 0.0 * C.xx ;
  // x1 = x0 - i1 + 1.0 * C.xx ;
  // x2 = x0 - 1.0 + 2.0 * C.xx ;
  vec4 x12 = x0.xyxy + C.xxzz;
  x12.xy -= i1;

// Permutations
  i = mod289_2_9(i); // Avoid truncation effects in permutation
  vec3 p = permute_2_10( permute_2_10( i.y + vec3(0.0, i1.y, 1.0 ))
    + i.x + vec3(0.0, i1.x, 1.0 ));

  vec3 m = max(0.5 - vec3(dot(x0,x0), dot(x12.xy,x12.xy), dot(x12.zw,x12.zw)), 0.0);
  m = m*m ;
  m = m*m ;

// Gradients: 41 points uniformly over a line, mapped onto a diamond.
// The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

  vec3 x = 2.0 * fract(p * C.www) - 1.0;
  vec3 h = abs(x) - 0.5;
  vec3 ox = floor(x + 0.5);
  vec3 a0 = x - ox;

// Normalise gradients implicitly by scaling m
// Approximation of: m *= inversesqrt( a0*a0 + h*h );
  m *= 1.79284291400159 - 0.85373472095314 * ( a0*a0 + h*h );

// Compute final noise value at P
  vec3 g;
  g.x  = a0.x  * x0.x  + h.x  * x0.y;
  g.yz = a0.yz * x12.xz + h.yz * x12.yw;
  return 130.0 * dot(m, g);
}



float smin_4_12(float a, float b, float k) {
  float h = clamp(0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
  return mix(b, a, h) - k * h * (1.0 - h);
}




// Taken from https://www.shadertoy.com/view/4ts3z2
// By NIMITZ  (twitter: @stormoid)
// good god that dudes a genius...

float tri( float x ){
  return abs( fract(x) - .5 );
}

vec3 tri3( vec3 p ){

  return vec3(
      tri( p.z + tri( p.y * 1. ) ),
      tri( p.z + tri( p.x * 1. ) ),
      tri( p.y + tri( p.x * 1. ) )
  );

}


float triNoise3D( vec3 p, float spd , float time){

  float z  = 1.4;
	float rz =  0.;
  vec3  bp =   p;

	for( float i = 0.; i <= 3.; i++ ){

    vec3 dg = tri3( bp * 2. );
    p += ( dg + time * .1 * spd );

    bp *= 1.8;
		z  *= 1.5;
		p  *= 1.2;

    float t = tri( p.z + tri( p.x + tri( p.y )));
    rz += t / z;
    bp += 0.14;

	}

	return rz;

}



mat3 calcLookAtMatrix( in vec3 ro, in vec3 ta, in float roll ) {
  float s = sin(0.0);
  float c = cos(0.0);
  vec3 ww = normalize( ta - ro );
  vec3 uu = normalize( cross(ww,vec3(s,c,0.0) ) );
  vec3 vv = normalize( cross(uu,ww));
  return mat3( uu, vv, ww );
}

vec3 doBackground(vec2 p) {
  vec3 col = vec3(0.0);

  col += snoise_1_4(vec3(p * 0.1, iGlobalTime)) * vec3(0.3, 0.1, 0.05);
  col += snoise_1_4(vec3(iGlobalTime, p * 0.4)) * vec3(0.1, 0.2, 0.4);
  return clamp(col, vec3(0.0), vec3(1.0));
}

float sdCone(vec3 p, vec2 c) {
  float q = length(p.xy);
  return dot(normalize(c),vec2(q,p.z));
}

float sdBox(vec3 p, vec3 b) {
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float sdCylinder(vec3 p, vec3 c) {
  return length(p.xz-c.xy)-c.z;
}

vec2 doModel(vec3 p) {
  float r = 0.5 + pow(triNoise3D(p * 0.5 + iGlobalTime * vec3(0, 0.1, 0), 1.5, iGlobalTime) * 0.1, 1.);
  float d = length(p) - r;

  return vec2(d, 0.0);
}

vec3 calcIntersection(vec3 ro, vec3 rd) {
  const float maxd = 20.0;
  const float prec = 0.001;
  float h = prec * 2.0;
  float t = +0.0;
  float r = -1.0;
  float k = -1.0;
  float g = 0.0;

  for (int i = 0; i < 90; i++) {
    if (h < prec || t > maxd) break;
    vec2 m = doModel(ro + rd * t);
    h = m.x;
    k = m.y;
    t += h;
    g += 0.025;
  }

  g = pow(g, 2.0);

  if (t < maxd) r = t;

  return vec3(r, k, g);
}

vec3 calcNormal(vec3 pos) {
  const float eps = 0.002;

  const vec3 v1 = vec3( 1.0,-1.0,-1.0);
  const vec3 v2 = vec3(-1.0,-1.0, 1.0);
  const vec3 v3 = vec3(-1.0, 1.0,-1.0);
  const vec3 v4 = vec3( 1.0, 1.0, 1.0);

	return normalize(
    v1*doModel( pos + v1*eps ).x +
    v2*doModel( pos + v2*eps ).x +
    v3*doModel( pos + v3*eps ).x +
    v4*doModel( pos + v4*eps ).x
 	);
}

vec3 doLighting(vec3 pos, vec3 nor, vec3 rd) {
  vec3 lig = vec3(0.0);
  
  vec3  dir1 = normalize(vec3(0.3, 0.9, 0.1));
  vec3  col1 = vec3(0.3, 0.5, 1.0);
  float dif1 = orenNayarDiffuse_3_0(dir1, normalize(-rd), nor, 0.5, 1.9);
  
  vec3  dir2 = normalize(vec3(0, -1, 0.5));
  vec3  col2 = vec3(0.4, 0.0, 0.2);
  float dif2 = orenNayarDiffuse_3_0(dir2, normalize(-rd), nor, 0.5, 1.9);
  
  lig += dif1 * col1;
  lig += dif2 * col2;
  lig += vec3(0.005, 0.03, 0.01);
  
  return lig;
}

void mainImage(out vec4 fragColor, in vec2 fragCoord) {
  vec2 p = (-iResolution.xy + 2.0*fragCoord.xy)/iResolution.y;
  vec3 color = doBackground(p);

  float an = 0.0; //9*iGlobalTime;
	vec3 camPos = vec3(3.5*sin(an),1.0,3.5*cos(an));
  vec3 camTar = vec3(0.0,0.0,0.0);
  mat3 camMat = calcLookAtMatrix(camPos, camTar, 0.0);

  vec3 ro = camPos;
  vec3 rd = normalize(camMat * vec3(p.xy, 2.0));

  vec3 t = calcIntersection(ro, rd);
  if (t.x > -0.5) {
    vec3 pos = ro + rd * t.x;
    vec3 nor = calcNormal(pos);

    color = mix(doLighting(pos, nor, rd), color, 0.0);
  }

  color += t.z * mix(vec3(0.1, 0.8, 1.5), vec3(1.5, 0, 0), 1.0 - (p.y + 0.7)) * pow(triNoise3D(vec3(p, iGlobalTime), 2.5, iGlobalTime) * 3.0 + 0.9, 0.3);
  color.g = smoothstep(-0.2, 1.1, color.g);
  color.r = smoothstep(0.1, 0.9, color.r);
    
  fragColor.rgb = color;
  fragColor.a   = 1.0;
}
