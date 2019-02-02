// https://www.shadertoy.com/view/XdXXW2
// Written by GLtracy

float tri( vec2 p0, vec2 p1, vec2 p2, vec2 v ) {
	vec2 e0 = p1 - p0;
	vec2 e1 = p2 - p1;
	vec2 e2 = p0 - p2;
	
	float len0 = length( e0 );
	float len1 = length( e1 );
	float len2 = length( e2 );
	
	e0 /= len0;
	e1 /= len1;
	e2 /= len2;
	
	p0 = v - p0;
	p1 = v - p1;
	p2 = v - p2;
	
	float h0 = dot( p0, vec2( e0.y, -e0.x ) );
	float h1 = dot( p1, vec2( e1.y, -e1.x ) );
	float h2 = dot( p2, vec2( e2.y, -e2.x ) );
	
	// cw
	if ( e0.x * e1.y - e0.y * e1.x < 0.0 ) {
		h0 = -h0;
		h1 = -h1;
		h2 = -h2;
	}
	
	// inside
	float m = max( max( h0, h1 ), h2 );
	if ( m <= 0.0 ) {
		return m;
	}
	
	// side
	vec3 t = vec3( dot( p0, e0 ), dot( p1, e1 ), dot( p2, e2 ) );
	
	vec3 s = step( 0.0, t ) * step( t, vec3( len0, len1, len2 ) );
	
	h0 *= s.x;
	h1 *= s.y;
	h2 *= s.z;
	
	float n = max( max( h0, h1 ), h2 );
	if ( n > 0.0 ) {
		return n;
	}
	
	// corner
	float d0 = length( p0 );
	float d1 = length( p1 );
	float d2 = length( p2 );
	return min( min( d0, d1 ), d2 );
}

// math
mat2 rot2( float angle ) {
	float c = cos( angle );
	float s = sin( angle );
	
	return mat2(
		 c, s,
		-s, c
	);
}

void main(void)
{
	vec2 v = ( gl_FragCoord.xy * 2.0 - iResolution.xy ) / min( iResolution.x, iResolution.y );
	
	mat2 r = rot2( sin( iGlobalTime * 0.5 ) * 3.14 );
	
	vec2 p0 = r * vec2( -1.3, -0.76 );
	vec2 p1 = r * vec2(  0.0,  0.73 );
	vec2 p2 = r * vec2(  1.3,  0.03 );
	
	vec2 p3 = vec2( -1.4,  0.76 ) * r;
	vec2 p4 = vec2(  0.6,  0.63 ) * r;
	vec2 p5 = vec2(  0.8, -0.58 ) * r;
	
	float d1 = tri( p0, p1, p2, v );
	float d2 = tri( p3, p4, p5, v );
	
	float d = max( d1, -d2 );
	const float band = 0.2;
	
	d = clamp( d / band, -1.0, +1.0 );
	
	gl_FragColor =
		vec4( 0.5, 0.7, 0.3, 0.0 ) * max( d, 0.0) +
		vec4( 0.2, 0.4, 0.7, 0.0 ) * max(-d, 0.0) +
		vec4( 1.0, 1.0, 1.0, 0.0 ) * ( 1.0 - smoothstep( 0.0, 0.04, abs( d ) ) );
}
