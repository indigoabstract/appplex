// https://www.shadertoy.com/view/lds3WH
// Code stolen from pouet.net and adapted
// I rewrote this to help a friend understand why it wasn't working with his version.
// Not intended to be a published shader at all in the first place! ^^
//
#define t iGlobalTime

float spikeball(vec3 p, vec3 v)
{
vec3 c[19];
c[0] = vec3(1,0,0);
c[1] = vec3(0,1,0);
c[2] = vec3(0,0,1);
c[3] = vec3(.577,.577,.577);
c[4] = vec3(-.577,.577,.577);
c[5] = vec3(.577,-.577,.577);
c[6] = vec3(.577,.577,-.577);
c[7] = vec3(0,.357,.934);
c[8] = vec3(0,-.357,.934);
c[9] = vec3(.934,0,.357);
c[10] = vec3(-.934,0,.357);
c[11] = vec3(.357,.934,0);
c[12] = vec3(-.357,.934,0);
c[13] = vec3(0,.851,.526);
c[14] = vec3(0,-.851,.526);
c[15] = vec3(.526,0,.851);
c[16] = vec3(-.526,0,.851);
c[17] = vec3(.851,.526,0);
c[18] = vec3(-.851,.526,0);

	float	MinDistance = 1e4;
	for ( int i=3; i < 19; i++ )
	{
		float	d = clamp( dot( p, c[i] ), -1.0, 1.0 );
		vec3	proj = d * c[i];
		d = abs( d );
		
		float	Distance2Spike = length( p - proj );
		float	SpikeThickness = 0.25 * exp( -5.0*d ) + 0.0;
		float	Distance = Distance2Spike - SpikeThickness;
		
		MinDistance = min( MinDistance, Distance );
	}
	
	return MinDistance;	
}

// Rotation
#define R(p, a) p=cos(a)*p+sin(a)*vec2(p.y,-p.x)

float Distance( vec3 p, vec3 v )
{
//	p.z += 10.0;
//	p.y += 0.5;
	vec3 q = p;
	R( p.yz, t );
	R( p.xz, 2.0*t + p.x*sin(t)*0.2 );
	float d = spikeball(p, v);
//return d;
	
	float nd = dot( q + vec3(0.0,3.0, 0.0), vec3(0.0, 1.0,0.0));
	return min( nd, d ) * 0.8;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec3 p = vec3( 0, 0, 20 );
	vec3 v = vec3( (2.0 * uv - 1.0) * vec2( iResolution.x / iResolution.y, 1 ), 0 ) - p;
	v = normalize(v);
  
  	// Raymarching loop
	float r, l, a, s, ml=0.001;
	for ( int i=0; i < 64; i++ )
	{
		l = Distance(p, v);
		p += l*v;
		l = abs(l);
		r += l;
		if (l < ml*r) break;
  }    
	
  // Compute normal
  vec2 epsilon = vec2( 0.01,0.0 );
  vec3 n=normalize(
    vec3(
      Distance(p+epsilon.xyy, v) - Distance(p-epsilon.xyy, v),
      Distance(p+epsilon.yxy, v) - Distance(p-epsilon.yxy, v),
      Distance(p+epsilon.yyx, v) - Distance(p-epsilon.yyx, v)
      )
    );

  gl_FragColor = vec4( 1.0+dot(n,v) );
}
