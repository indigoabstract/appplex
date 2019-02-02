// https://www.shadertoy.com/view/Xds3zM
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

float hash( float n )
{
    return fract(sin(n)*43758.5453123);
}

vec2 distanceToSegment( vec2 a, vec2 b, vec2 p )
{
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	return vec2( length( pa - ba*h ), h );
}

struct Segment
{
	vec2 p;
	float a;
	float l;
	float wa;
	float wb;
	int level;
};
	
void main(void)
{
	vec2 p = (-iResolution.xy + 2.0*gl_FragCoord.xy) / iResolution.y;
	vec2 m = vec2(0.0);
	if( iMouse.z>0.0 ) m=-1.0 + 2.0*iMouse.xy/ iResolution.xy;
		
	Segment stack[64];
	stack[0] = Segment( vec2(0.0,-0.85), 0.0, 0.6, 0.13, 0.08, 0 );
	int s = 0;
	
	float id = 0.0;
	float f = 100.0;
	float g = 100.0;
	for( int i=0; i<63; i++ )
	{
		/// pop from the stack
		Segment x = stack[s--];
		
		// render line
		vec2 a = x.p;
		vec2 b = x.p + x.l*vec2(sin(x.a),cos(x.a));
		vec2 h = distanceToSegment( a, b, p );
		float d = h.x - mix(x.wa,x.wb,h.y);
		f = min( f,     d);
		g = min( g, abs(d) );

        // push new stuff into the stack		
		id += 1.0;
		if( x.level<5 )
		{
			float an = m.x + 0.5*sin(8.0*x.l+iGlobalTime)*x.l;
			float a1 = 0.2 + 0.8*hash(3313.115*id) + an;
			float a2 = 0.2 + 0.8*hash(1241.506*id) - an;
			float l1 = 0.5 + 0.3*hash(5241.343*id);
			float l2 = 0.5 + 0.3*hash(9741.241*id);
			stack[++s] = Segment( b, x.a+a1, x.l*l1, x.wb, x.wb*0.65, x.level+1 );
			stack[++s] = Segment( b, x.a-a2, x.l*l2, x.wb, x.wb*0.65, x.level+1 );
		}
	}

    // final color
	vec3 col = 0.2 + 0.6*vec3( sqrt(abs(f)) );
	col = mix( vec3(1.0,0.7,0.3), col, smoothstep( 0.0, 0.005, f ) );	
  	col *= smoothstep( 0.0, 0.01, g );
  	
	gl_FragColor = vec4(col,1.0);
}
