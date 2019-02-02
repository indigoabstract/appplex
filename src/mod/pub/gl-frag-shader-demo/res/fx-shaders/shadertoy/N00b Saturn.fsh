// https://www.shadertoy.com/view/4dl3RS

// iChannel0: t14

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// Edited by Steven An - 2013

//#define STEREO 
vec3 mod289(vec3 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
     return mod289(((x*34.0)+1.0)*x);
}

vec3 permute(vec3 x) {
  return mod289(((x*34.0)+1.0)*x);
}

float snoise2d(vec2 v)
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
  i = mod289(i); // Avoid truncation effects in permutation
  vec3 p = permute( permute( i.y + vec3(0.0, i1.y, 1.0 ))
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


float noise( in vec3 x )
{
    vec3 p = floor(x);
    vec3 f = fract(x);
	f = f*f*(3.0-2.0*f);
	
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel0, (uv+ 0.5)/256.0, -100.0 ).yx;
	return mix( rg.x, rg.y, f.z );
}

float noise( in vec2 x )
{
    vec2 p = floor(x);
    vec2 f = fract(x);
	vec2 uv = p.xy + f.xy*f.xy*(3.0-2.0*f.xy);
	return texture2D( iChannel0, (uv+118.4)/256.0, -100.0 ).x;
}

#ifdef STEREO
#define lodbias -5.0
#else
#define lodbias 0.0
#endif

vec4 texcube( sampler2D sam, in vec3 p, in vec3 n )
{
	vec4 x = texture2D( sam, p.yz, lodbias );
	vec4 y = texture2D( sam, p.zx, lodbias );
	vec4 z = texture2D( sam, p.xy, lodbias );
	return x*abs(n.x) + y*abs(n.y) + z*abs(n.z);
}

//=====================================================================

float lava( vec2 p )
{
	p += vec2(2.0,4.0);
    float f;
    f  = 0.5000*noise( p ); p = p*2.02;
    f += 0.2500*noise( p ); p = p*2.03;
    f += 0.1250*noise( p ); p = p*2.01;
    f += 0.0625*noise( p );
    return f;
}

// I think this is just a rotation matrix - haven't analyzed it really.
const mat3 m = mat3( 0.00,  0.80,  0.60,
                    -0.80,  0.36, -0.48,
                    -0.60, -0.48,  0.64 );

float displacement( vec3 p )
{
//	p += vec3(1.0,0.0,0.8);
	
    float f;
    f  = 0.5000*noise( p ); p = m*p*2.02;
    f += 0.2500*noise( p ); p = m*p*2.03;
    f += 0.1250*noise( p ); p = m*p*2.01;
    f += 0.0625*noise( p ); 
	
	// This just adds more high frequency noise to the surface. 
	float n = noise( p*5.5 );
    f += 0.03*n*n;
	
    return f;
}

float ringsSDF( in vec3 pos )
{	
	// XZ-slab
	const float height = 0.1;
	float slabSDF = abs(pos.y)-height;
	
	// y-infinite rings
	const float ringSpacing = 3.0;
	float xzDist = fract(length(pos.xz)/ringSpacing);
	float ringsSDF = -abs(xzDist-0.5) + 0.25;
	
	// noise up the rings to make them look like rocks
	float waves = 0.25*noise(10.0*pos) + 0.01*noise(100.0*pos);
	
	// intersect with max
	return max( slabSDF, ringsSDF ) + waves;
}

float planetSDF( in vec3 p )
{
	const float radius = 5.0;
	const float bandFreq = 8.0;
	vec3 spotDir = normalize(vec3(1.0,0.5,0.0));
	float noisePart = 0.1 * noise(5.0*p) * sin(3.0*bandFreq*dot(normalize(p),spotDir));
	return length(p) - radius + noisePart;
}

// This is the main signed distance function for the terrain.
// The higher this value goes, the further it is from a surface.
float mapTerrain( in vec3 pos )
{
	return min( ringsSDF(pos), planetSDF(pos));
	
//	return pos.y*0.1 + 0.05*sin(length(pos));
	
	return pos.y*0.4 + (0.5*noise(pos)) * (1.0-smoothstep(1.0,3.0,pos.y));
	
	return pos.y*0.2
		+ (displacement(pos*vec3(0.8,1.0,0.8)) - 0.4)*(1.0-smoothstep(1.0,3.0,pos.y));

}

float raymarchTerrain( in vec3 ro, in vec3 rd )
{
	float maxd = 30.0;
	float precis = 0.001;
    float distToSurf = 1.0;
    float t = 0.1;
    for( int i=0; i<160; i++ )
    {
        if( abs(distToSurf)<precis||t>maxd ) continue;//break;
	    distToSurf = mapTerrain( ro+rd*t );
        t += distToSurf;
    }

    if( t>maxd ) t=-1.0;
    return t;
}

vec3 calcNormal( in vec3 pos )
{
    vec3 eps = vec3(0.02,0.0,0.0);
	return normalize( vec3(
           mapTerrain(pos+eps.xyy) - mapTerrain(pos-eps.xyy),
           mapTerrain(pos+eps.yxy) - mapTerrain(pos-eps.yxy),
           mapTerrain(pos+eps.yyx) - mapTerrain(pos-eps.yyx) ) );

}

vec3 lig = normalize( vec3(-0.3,0.4,0.7) );
	
vec4 mapClouds( in vec3 pos )
{
	vec3 q = pos*0.5 + vec3(0.0,-iGlobalTime,0.0);
	
	float d;
    d  = 0.5000*noise( q ); q = q*2.02;
    d += 0.2500*noise( q ); q = q*2.03;
    d += 0.1250*noise( q ); q = q*2.01;
    d += 0.0625*noise( q );
		
	d = d - 0.55;
	d *= smoothstep( 0.5, 0.55, lava(0.1*pos.xz)+0.01 );

	d = clamp( d, 0.0, 1.0 );
	
	vec4 res = vec4( d );

	res.xyz = mix( vec3(1.0,0.8,0.7), 0.2*vec3(0.4,0.4,0.4), res.x );
	res.xyz *= 0.25;
	res.xyz *= 0.5 + 0.5*smoothstep( -2.0, 1.0, pos.y );
	
	return res;
}

vec4 raymarchClouds( in vec3 ro, in vec3 rd, in vec3 bcol, float tmax )
{
	vec4 sum = vec4( 0.0 );

	float sun = pow( clamp( dot(rd,lig), 0.0, 1.0 ),6.0 );
	float t = 0.0;
	for( int i=0; i<60; i++ )
	{
		if( t>tmax || sum.w>0.95 ) continue;
		vec3 pos = ro + t*rd;
		vec4 col = mapClouds( pos );
		
        col.xyz += vec3(1.0,0.7,0.4)*0.4*sun*(1.0-col.w);
		col.xyz = mix( col.xyz, bcol, 1.0-exp(-0.00006*t*t*t) );
		
		col.rgb *= col.a;

		sum = sum + col*(1.0 - sum.a);	

		t += max(0.1,0.05*t);
	}

	sum.xyz /= (0.001+sum.w);

	return clamp( sum, 0.0, 1.0 );
}

float softshadow( in vec3 ro, in vec3 rd, float mint, float k )
{
    float res = 1.0;
    float t = mint;
    for( int i=0; i<48; i++ )
    {
        float h = mapTerrain(ro + rd*t);
		h = max( h, 0.0 );
        res = min( res, k*h/t );
        t += clamp( h, 0.02, 0.5 );
    }
    return clamp(res,0.0,1.0);
}

vec3 path( float time )
{
	return vec3( 16.0*cos(0.2+0.5*.1*time*1.5), 1.5, 16.0*sin(0.1+0.5*0.11*time*1.5) );
	
}

void main( void )
{
	#ifdef STEREO
	float eyeID = mod(gl_FragCoord.x + mod(gl_FragCoord.y,2.0),2.0);
    #endif

    vec2 q = gl_FragCoord.xy / iResolution.xy;
	vec2 p = -1.0 + 2.0*q;
	p.x *= iResolution.x / iResolution.y;
	
	
    // camera	
	float time = 2.7+iGlobalTime;
	// TEMP
	vec3 ro = path( time+0.0 );
	vec3 camTarget = path( time+1.6 );
	//camTarget.y *= 0.3 + 0.25*cos(0.11*time);
	camTarget.y *= 0.35 + 0.25*sin(0.09*time);
	float roll = 0.3*sin(1.0+0.07*time);
	
	// TEMP
	//roll = 0.0;
	camTarget = vec3(0,0,0);
	
	// stop going into the planet
	ro = normalize(ro)*max( 10.0, length(ro));
	
	// camera tx
	vec3 cw = normalize(camTarget-ro);
	vec3 cp = vec3(sin(roll), cos(roll),0.0);
	vec3 cu = normalize(cross(cw,cp));
	vec3 cv = normalize(cross(cu,cw));
	
	float r2 = p.x*p.x*0.32 + p.y*p.y;
    p *= (7.0-sqrt(37.5-11.5*r2))/(r2+1.0);

	vec3 rd = normalize( p.x*cu + p.y*cv + 2.1*cw );

	#ifdef STEREO
	vec3 fo = ro + rd*7.0; // put focus plane behind Mike
	ro -= 0.2*cu*eyeID;    // eye separation
	rd = normalize(fo-ro);
    #endif

    // sky	 
	vec3 col = vec3(0.01,0.01,0.05) - rd.y*0.4;
    float sun = clamp( dot(rd,lig), 0.0, 1.0 );
	col += vec3(1.0,0.8,0.4)*0.2*pow( sun, 6.0 );
    col *= 0.9;

	vec3 bcol = col;
    

    // terrain	
	float t = raymarchTerrain(ro, rd)+0.0;

    if( t>0.0 )
	{
		vec3 pos = ro + t*rd;
		vec3 nor = calcNormal( pos );
		vec3 ref = reflect( rd, nor );	// not used?
		
		// Perturb the normal by sampling 3D noise
		// This is largely responsible for the rough, crumbly look of the volcanic rock. Very cool!
		vec3 bn = -1.0 + 2.0*texcube( iChannel0, 3.0*pos/4.0, nor ).xyz;
		nor = normalize( nor + 0.6*bn );
		
		float hh = 1.0 - smoothstep( -2.0, 1.0, pos.y );

        // lighting
		float dif = sqrt(clamp( dot( nor, lig ), 0.0, 1.0 ));
		float sha = 0.0;
		if( dif>0.01)
			sha=softshadow(pos,lig,0.01,32.0);
		float bac = clamp( dot( nor, normalize(lig*vec3(-1.0,0.0,-1.0)) ), 0.0, 1.0 );
		float sky = 0.5 + 0.5*nor.y;
        float lav = smoothstep( 0.5, 0.55, lava(0.1*pos.xz) )*hh*clamp(0.5-0.5*nor.y,0.0,1.0);
		float occ = pow( (1.0-displacement(pos*vec3(0.8,1.0,0.8)))*1.6-0.5, 2.0 );

		float amb = 1.0;

		col = vec3(0.8);

		// Here I've simplified the BRDF to only include the diffuse and ambient terms.
		vec3 albedo = mix( vec3(1,0,0), vec3(0,1,0), snoise2d( pos.xy ) );
		vec3 brdf =	dif * albedo;
		brdf += 0.1 * amb * vec3(1,1,1);
		
	//	vec3 brdf  = 1.3*dif*vec3(0.9,0.70,0.55)*sha*(0.8+0.2*occ);
		//     brdf += 0.8*bac*vec3(0.5,0.35,0.25)*occ;
		  //   brdf += 0.8*sky*vec3(0.2,0.25,0.35)*occ;
		  //   brdf += 0.6*amb*vec3(0.3,0.25,0.25)*occ;
		     brdf += 0.5*lav*vec3(2.0,0.41,0.00);


/*
        // surface shading/material		
		col = texcube( iChannel1, 0.5*pos, nor ).xyz;
		col = col*(0.2+0.8*texcube( iChannel2, 4.0*vec3(2.0,8.0,2.0)*pos, nor ).x);
*/
		
/*
The "grass" effect

		vec3 verde = vec3(1.0,0.9,0.2);
		verde *= texture2D( iChannel2, pos.xz, lodbias ).xyz;
		col = mix( col, 0.8*verde, hh );
		
		float vv = smoothstep( 0.0, 0.8, nor.y )*smoothstep(0.0, 0.1, pos.y-0.8 );
		verde = vec3(0.2,0.45,0.1);
		verde *= texture2D( iChannel2, 30.0*pos.xz, lodbias ).xyz;
		verde += 0.2*texture2D( iChannel2, 1.0*pos.xz, lodbias ).xyz;
		vv *= smoothstep( 0.0, 0.5, texture2D( iChannel2, 0.1*pos.xz + 0.01*nor.x ).x );
		col = mix( col, verde*1.1, vv );
*/
		
        // light/surface interaction		
		col = brdf * col;
		
		// atmospheric
		col = mix( col, (1.0-0.7*hh)*bcol, 1.0-exp(-0.00006*t*t*t) );
	}
	
	// A bunch of other cool stuff is done down here. I commented it out, to study
	// the terrain in isolation.

	// sun glow
    col += vec3(1.0,0.6,0.2)*0.23*pow( sun, 2.0 )*clamp( (rd.y+0.4)/(0.0+0.4),0.0,1.0);
	
    // smoke	
/*	
	{
	if( t<0.0 ) t=600.0;
    vec4 res = raymarchClouds( ro, rd, bcol, t );
	col = mix( col, res.xyz, res.w );
	}
	*/

    // gamma	
//	col = pow( clamp( col, 0.0, 1.0 ), vec3(0.45) );

    // contrast, desat, tint and vignetting	
	/*
	col = col*0.3 + 0.7*col*col*(3.0-2.0*col);
	col = mix( col, vec3(col.x+col.y+col.z)*0.33, 0.2 );
	col *= 1.3*vec3(1.06,1.1,1.0);
	col *= 0.5 + 0.5*pow( 16.0*q.x*q.y*(1.0-q.x)*(1.0-q.y), 0.1 );
	*/

    #ifdef STEREO	
    col *= vec3( eyeID, 1.0-eyeID, 1.0-eyeID );	
	#endif
	
	gl_FragColor = vec4( col, 1.0 );
}
