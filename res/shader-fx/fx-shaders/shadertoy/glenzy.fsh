// https://www.shadertoy.com/view/ldlSW2
vec3 tc;
float Cube( vec3 p )
{
    float Time = 2.1 + iGlobalTime*0.6;
	tc.x = sin( Time );
	tc.y = cos( Time );
	mat2 m = mat2( tc.y, -tc.x, tc.x, tc.y );
	p.xy *= m;
	p.xy *= m;
	p.yz *= m;
	p.zx *= m;
	p.zx *= m;
	p.zx *= m;
	tc = p;
	return length( max( abs( p ) - 0.5, 0.0 ) );
}

float Text( vec2 uv )
{
	uv.y = mod( uv.y, 1.0 );
	return ( ( uv.y < uv.x ) != ( 1.0 - uv.y < uv.x ) ) ? 1.0 : 0.0;
}


void main()
{

	vec2 p = -1.0 + 2.0*gl_FragCoord.xy / iResolution.xy;
	p.x *= iResolution.x / iResolution.y;


	vec3 ro = vec3( 0.0, 0.0, 1.5 );
	vec3 rd = normalize( vec3( p, -1.0 ) );

	float Step = 1.0;
	float Distance = 0.0;

	float Near = -1.0;
	float Far = -1.0;
    
	for( int i = 0; i < 120; i++ )
	{
		if( Distance > 4.0 )
			break;

		if( Step < 0.001 )
		{
			Far = Text( tc.yx ) + Text( -tc.yx ) + Text( tc.xz ) + Text( -tc.xz ) + Text( tc.zy ) + Text( -tc.zy );
			Distance += 0.013;

			if( Near < 0.0 )
			{
				Near = Far;
			}
		}



		Step = Cube( rd*Distance + ro );
		Distance += Step;
	}

	vec3 Color = vec3( 0.0 );

	if( Near > 0.0 )
	{
		Color = mix( vec3( 1.0, 0.0, 0.0 ), vec3( 1.0, 1.0, 1.0 ), vec3( ( Near*0.45 + Far*Far*0.04 ) ) );
	}

	gl_FragColor = vec4( Color, 1.0 );
}
