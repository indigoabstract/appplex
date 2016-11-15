// https://www.shadertoy.com/view/MsfXW7
// Terrain marcher made for a computer graphics course. Click and hold to move light source
// Raymarching steps
#define STEP 0.05
#define TMIN 1.0
#define TMAX 4.5
// Partial derivative step
#define DSTEP 0.01

// Colors
#define DIFFUSE vec4(1,0,0,1)
#define SPECULAR vec4(1,1,1,1)
#define AMBIENT vec4(0.2,0.2,0.2,1)
#define LIGHT vec4(0.9,0.8,0,1)
#define SHINE 100.0

// Fbm parameters
#define OCTAVES 6
#define GAIN 0.5
#define LACUNARITY 2.0

precision highp float;

struct Ray {
    vec3 origin;
    vec3 direction;
};

// GLSL textureless classic 2D noise "cnoise",
// with an RSL-style periodic variant "pnoise".
// Author:  Stefan Gustavson (stefan.gustavson@liu.se)
// Version: 2011-08-22
//
// Many thanks to Ian McEwan of Ashima Arts for the
// ideas for permutation and gradient selection.
//
// Copyright (c) 2011 Stefan Gustavson. All rights reserved.
// Distributed under the MIT license. See LICENSE file.
// https://github.com/ashima/webgl-noise
//
// Copyright (C) 2011 by Ashima Arts (Simplex noise)
// Copyright (C) 2011 by Stefan Gustavson (Classic noise)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
  return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(vec2 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}

// Classic Perlin noise
float cnoise(vec2 P)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod289(Pi); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;

  vec4 i = permute(permute(ix) + iy);

  vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
  vec4 gy = abs(gx) - 0.5 ;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;

  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);

  vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
  g00 *= norm.x;  
  g01 *= norm.y;  
  g10 *= norm.z;  
  g11 *= norm.w;  

  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));

  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}

// Fractal Brownian Motion
float fbm(vec2 P)
{
  float total = 0.0;
  float frequency = 1.0;
  float amplitude = GAIN;

  for (int i = 0; i < OCTAVES; i++)
  {
    float noise = cnoise( P * frequency );
    total += (noise + 1.0) / 2.0 * amplitude;         
    frequency *= LACUNARITY;
    amplitude *= GAIN;
  }

  return total;
}

float map(vec2 P)
{
  return fbm(P + iGlobalTime/5.0);
}
/////////////////////////////////////////////////////////////////////

// Ray marching
float intersect(Ray r)
{
  float lastError = 0.0;

  for (float t = TMIN; t < TMAX; t += STEP)
  {
    vec3 p = r.origin + t*r.direction;
    float h = map(p.xy);
    float error = p.z - h;
    
    if( error < 0.0 ) // Hit
    {
      float alpha = error/(lastError - error);
      return t + alpha*STEP;
    }
    else
    {
      lastError = error;
    }
  }
  return 0.0;
}

vec3 calcNorm(vec2 P)
{
  vec2 dx = vec2(DSTEP/2.0,0);
  vec2 dy = vec2(0,DSTEP/2.0);
  float partialx = (map(P + dx) - map(P - dx))/DSTEP;
  float partialy = (map(P + dy) - map(P - dy))/DSTEP;
  return normalize(vec3(-partialx, -partialy, 1));
}

vec4 shade(vec2 P, vec3 dir, vec3 light)
{
  vec3 l = normalize(light - vec3(P, map(P)));
  vec3 n = calcNorm(P);
  float nl = max(0.0, dot(n,l));

  vec3 h = normalize(-dir + l);
  float nh = max(0.0, pow(dot(n,h), SHINE));

  return DIFFUSE*AMBIENT + LIGHT*(DIFFUSE*nl + SPECULAR*nh);
}

void main(void)
{
  vec2 pos = gl_FragCoord.xy/iResolution.xy;
  vec3 u = normalize(vec3(0, 1, -1));
  vec3 v = vec3(1,0,0);
  vec3 w = normalize(vec3(0,1,1));
  vec3 camera = -2.0*u;
  vec3 dir = u + 0.7*(pos.x*v + pos.y*w);
  Ray r = Ray(camera, dir);
  vec3 light = vec3((4.0*iMouse.xy/iResolution.xy) - vec2(1,1),1);
  float t = intersect(r);

  if(t == 0.0)
  {
    gl_FragColor = vec4(0,0,0,1);
  }
  else
  {
    gl_FragColor = shade((r.origin + r.direction*t).xy, r.direction, light);
  }
}
