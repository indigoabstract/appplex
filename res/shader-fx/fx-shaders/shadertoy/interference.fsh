// https://www.shadertoy.com/view/XslGzH
#define PI 3.1415926

float distanceToSegment( vec2 a, vec2 b, vec2 p )
{
	vec2 pa = p - a;
	vec2 ba = b - a;
	float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
	
	return length( pa - ba*h );
}

void main(void)
{
	float time = 100.0+iGlobalTime;
	vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.yy;
	p.x -= 0.66;
	float r = dot( p, p ), a = atan( p.x, p.y );
	
	float f = smoothstep( 0.0, 1.0, r );
	float s = mix( 0.5, f, abs(a/(2.*PI)) );
	vec3 col = vec3( 0.5+0.5*cos(a)*r/1.2, s, 0.6-0.4*sin(a) );
	//vec3 col = vec3( 0.0, 1.0-s, s );

	float d = a+r*(0.3+0.2*sin(3.0*time*r))*40.0-5.0*time;
	vec2 pd = vec2( r/5.0*sin(d), r*cos(d) );
	f = distanceToSegment( vec2(0.0), vec2(10.0, 0), pd );
	s = smoothstep( r/2.0, r/2.0*1.01, f );
	//col = mix( col, vec3(s), 0.8 );
	col *= vec3( 1.0-s );
		
	
	gl_FragColor = vec4( col ,1.0 );
}
