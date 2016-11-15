// https://www.shadertoy.com/view/MdjGWt
// KALI FRACTAL
vec3 kali( vec3 v , vec3 s ){
	float m = 0.0;
	for( int i = 0; i < 5 ; i ++){
	v.x = abs(v.x);
	v.y = abs(v.y); 
	v.z = abs(v.z);
	m = v.x * v.x + v.y * v.y + v.z * v.z;
	v.x = v.x / m + s.x;
	v.y = v.y / m + s.y;
	v.z = v.z / m + s.z;
    }
	return v;
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





float r;

float map( in vec3 p )
{
	

	vec4 a = texture2D(iChannel0 ,  vec2(abs(p.x) , abs(p.y) ));
	float t1 = .5 + .5*sin(  iGlobalTime * .3);
	float t2 = .5 + .5*sin(  iGlobalTime * .2);
	float t3 = .5 + .5*sin(  iGlobalTime * .1);
	
	//p *= t1;
	float noise = .9 * snoise( vec3( p.x , p.y , t1 ));
	float noise1 = .9 * snoise( vec3( p.x * 5. , p.y * 5. , t2 ));
	float noise2 = .9 * snoise( vec3( p.x * 10. , p.y * 10. , t3 ));
	float noise3 = .9 * snoise( vec3( p.x * 40. , p.y * 40. , t1 ));

	
	//float noise2
	
	float e = r * noise*noise1*noise2*noise3;
	
    return  max( e + a.x * .01 + length(p) -1.0 , -2.0 );
}

vec3 calcNormal( in vec3 p )
{

	vec3 e = vec3(0.001,0.0,0.0);
	return normalize( vec3(map(p+e.xyy) - map(p-e.xyy),
						   map(p+e.yxy) - map(p-e.yxy),
						   map(p+e.yyx) - map(p-e.yyx) ) );
						   
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	vec2 p = -1.0 + 2.0*uv;
	
	p.x *= iResolution.x/iResolution.y;

    //r = 2.0*texture2D( iChannel0, uv ).x;
	r = 1.0;
	vec3 ro = vec3(0.0, 0.0, 2.0 );
	vec3 rd = normalize( vec3( p, -1.0 ) );
	
	
	vec3 col = vec3(0.0);
	
	
	float tmax = 10.0;
	float h = 1.0;
	float t = 0.0;
	for( int i=0; i<100; i++ )
	{
		if( h<0.00001 || t>tmax ) continue;
		h = map( ro + t*rd );
		t += h;
	}
	
	
	if( t<tmax )
	{
		vec3 pos = ro + t*rd;
		vec3 nor = calcNormal( pos );
		col = vec3(1.0, 1.0, 1.0 );
		//col *= dot( nor, vec3(0.57703) );
		
		vec3 nReduce = nor * .4;
		vec3 c = nReduce + vec3( .5 , .5 , .5 );
		vec3 kCol = kali( vec3( -c.x  , -c.y , -c.z ) , vec3( -.5 , -.5 , -.5 ) );
		col = normalize( vec3( .5  , .4 , .9 ) + kCol );
		col *= dot( nor, vec3(0.57703) );
	}

	gl_FragColor = vec4(col,1.0);
}
