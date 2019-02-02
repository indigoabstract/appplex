// https://www.shadertoy.com/view/lssXR4

// iChannel0: m0
// iChannel1: t12

// mi-ku / altair

#define STEPS_1 15
#define STEPS_2 50

float pyramid( vec3 pos, float modd, float zshift, float sp )
{
	return min(
		( 1.0 - abs( mod( pos.x, modd ) - 1.0 ) ), ( 1.0 - abs( mod( pos.z * 1.0 + zshift, modd ) - 1.0 ) ) ) *
		( 1.0 + sp * 4.0 );

}

vec3 mapGround( vec3 pos, float sp )
{
	pos *= 0.35;
	float pm = 0.6;
	float modd = 2.0;

	float zshift = -iGlobalTime * 4.0;
	float h = pyramid( pos, modd, zshift, sp );
	float dm = max( 0.0, 1.0 - max( 0.0, length( pos ) * 0.03 ) );
	dm *= max( 0.0, 1.0 - abs( pos.x ) * .2 );

	return vec3( pos.x,
				//( sin( pos.x * pm ) + sin( pos.z * pm + iGlobalTime * 10.0 ) ) * (0.5+sp*2.0) - 10.0,
				h * dm * 1.5 - 10.,
				pos.z );
}

float rmg( vec3 ro, vec3 rd, float sp )
{
	float t = 0.0;
	for( int i = 0; i < STEPS_2; i++ )
	{
		vec3 pt = ro + rd * t;
		float h = abs( pt.y - mapGround( pt, sp ).y );
		if ( h < 0.1 )
		{
			break;
		}
		t += 0.3 * h;
	}
	return t;
}

float rmss( vec3 pos, vec3 spos, float sp )
{
	return length( pos - spos ) - 5.0 - pyramid( pos.xzy, 2.0, 0.0, sp ) * sp -sp * 4.0;
}

float rms( vec3 ro, vec3 rd, vec3 spos, float sp )
{
	vec3 pt = ro;
	float t = 0.0;
	for( int i = 0; i < STEPS_1; i++ )
	{
		pt += rd * rmss( pt, spos, sp ) * 0.9;
	}
	return length( pt - ro );
}

vec3 shadeSphere( vec3 eye, vec3 pt, vec3 norm, vec3 light, float mult )
{
	vec3 r = normalize( reflect( light, norm ) );
	vec3 eyeDir = normalize( pt - eye );
	float diffuse = max( 0.0, dot( mult * r, eyeDir ) + 1.0 ) * 0.5;
	float spec = max( 0.0, dot( norm, eyeDir ) );
	return
		vec3( 1.0, 0.1, 0.01 )
		//vec3( 1.0, 1.0, 1.0 )
		* ( diffuse + spec );
}

vec3 shadeBG( vec2 uv, float sp )
{
	vec3 color = vec3( 0.7, 0.2, 0.1 );
	float h = pyramid( vec3( uv.x * 10.0, 0.0, 0.0 + 1.2 - sp ), 2.0, 0.0, 0.0 );
	h = max( 0.7, min( 1.0, 1.0 - ( uv.y - h * 0.1 ) * 50.0 ) );
	return color * h;
}

vec3 colorize( vec2 uv )
{
	vec3 ro = vec3( 0.0, 0.0, 0.0 );
	vec3 rd = vec3( uv.x, uv.y, 1.0 );
	rd = normalize( rd );

	vec3 spos = vec3( 0.0, sin( iGlobalTime * 2.0 ) * 5.0 + 10.0, 50.0 + cos( iGlobalTime * 2.0 ) * 10.0 );
	float sp = texture2D( iChannel0, vec2( 0.0, 0.0 ) ).r;

	float t = rmg( ro, rd, sp );
	float ts = rms( ro, rd, spos, sp );
	vec3 light = vec3( 0.0, 1.0, 0.0 );

	vec3 color;
	if ( t < ts && uv.y < 0.0 )
	{
		vec3 pt = ro + rd * t;

		float eps = 0.1;
		vec3 norm = vec3( mapGround( pt + vec3( eps, 0.0, 0.0 ), sp ).y - mapGround( pt, sp ).y,
						  0.3,
						  mapGround( pt + vec3( 0.0, 0.0, eps ), sp ).y - mapGround( pt, sp ).y );
		    norm += vec3( mapGround( pt + vec3( eps * 2.0, 0.0, 0.0 ), sp ).y - mapGround( pt +  vec3( eps, 0.0, 0.0 ), sp ).y,
						  0.3,
						  mapGround( pt + vec3( 0.0, 0.0, eps ), sp ).y - mapGround( pt + vec3( 0.0, 0.0, eps * 0.5 ), sp ).y );
		    norm += vec3( mapGround( pt + vec3( eps * 2.0, 0.0, 0.0 ), sp ).y - mapGround( pt +  vec3( eps, 0.0, 0.0 ), sp ).y,
						  0.3,
						  mapGround( pt + vec3( 0.0, 0.0, eps * 2.0 ), sp ).y - mapGround( pt + vec3( 0.0, 0.0, eps ), sp ).y );
		norm = normalize( norm );
		
		vec3 rd2 = normalize( -reflect( pt - ro, norm ) );
		float ts2 = rms( pt, rd2, spos, sp );
		vec3 npt = pt + rd2 * ts2;
		//if ( ts2 > 10.0 )
		{
			norm = normalize( npt - spos ) * 0.01;
		}
		color = shadeSphere( pt, npt, norm, light, -1.0 );
	}
	else
	{
		if ( ts < 100.0 )
		{
			vec3 pt = ro + rd * ts;
			vec3 norm = normalize( pt - spos );

			color = shadeSphere( ro, pt, norm, light, 1.0 );
		}
		else
		{
			color = shadeBG( uv, sp );
		}
	}

	return color;
}

vec3 noiseGrain( vec2 uv )
{
	return vec3(
		texture2D( iChannel1, uv * 5.0 + vec2( iGlobalTime * 10.678, iGlobalTime * 10.317 ) ).r
	) * 0.3;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv -= vec2( 0.5, 0.5 );
	float dist = ( 0.6 - length( uv ) * 0.7) * 15.0;
	vec3 color = colorize( uv ) * dist - noiseGrain( uv );

	gl_FragColor = vec4(color,1.0);
}
