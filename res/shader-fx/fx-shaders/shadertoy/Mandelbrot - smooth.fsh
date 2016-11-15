// https://www.shadertoy.com/view/4df3Rn
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

void main( void )
{
    vec2 p = (-iResolution.xy + 2.0*gl_FragCoord.xy)/iResolution.y;

    float zoo = .62+.38*cos(.05*iGlobalTime);
    float coa = cos( 0.1*(1.0-zoo)*iGlobalTime );
    float sia = sin( 0.1*(1.0-zoo)*iGlobalTime );
    zoo = pow( zoo,8.0);
    vec2 xy = vec2( p.x*coa-p.y*sia, p.x*sia+p.y*coa);
    vec2 cc = vec2(-.745,.186) + xy*zoo;

    float co = 0.0;
	vec2 z  = vec2(0.0);
    for( int i=0; i<256; i++ )
    {
        // z = z*z + c		
		z = vec2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y ) + cc;
		
		if( dot(z,z)>(256.0*256.0) ) break;

		co += 1.0;
    }

	// ------------------------------------------------------
    // smooth interation count
	// float sco = co - log2(log(length(z))/log(256.0));
	
    // equivalent optimized smooth interation count
	float sco = co - log2(log2(dot(z,z))) + 4.0;
	// ------------------------------------------------------
	


	float al = smoothstep( -0.1, 0.0, sin(0.5*6.2831*iGlobalTime ) );
    co = mix( co, sco, al );

	gl_FragColor = vec4( 0.5 + 0.5*cos( 3.0 + co*0.2 + vec3(0.0,0.5,1.0)), 1.0 );
}
