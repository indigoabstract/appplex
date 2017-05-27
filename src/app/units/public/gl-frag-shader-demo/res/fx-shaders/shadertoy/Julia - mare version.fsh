// https://www.shadertoy.com/view/XdlGRr
void main(void)
{
	vec2 p = gl_FragCoord.xy / iResolution.xy;
	
	
	float modeTimer = iGlobalTime;

	float vignetteVal = 1.5 - 2.0 * length( p - vec2( .5, .5 ) );
	float progress = 1.0 - fract( modeTimer * .05 );//.5 + .5 * cos( modeTimer * 0.15 );
	progress *= 1.0;
	float phase = pow( progress, 6.0 ) * 1.0;
	float scale = pow( progress, 6.0 ) * .4  + .00003;
	float sinuses = pow( 1.0 - progress, 0.5 );
	float angle = pow( progress, 3.0 ) * 4.0;
	vec2 rot = vec2( sin(angle), -cos( angle) );
	p = (-1.5 + 3.0*p)*vec2(1.7,1.0);
	
	vec2 pRotated = vec2( rot.x * p.x - rot.y * p.y, rot.y * p.x + rot.x * p.y );
	
	float fractZScale = 1.0 + pow( progress, 1.5 ) * 19.0;
	sinuses = -.3 + 5.8 * sinuses;
	//sinuses = .3 - 20.0 * progress * (progress - 1.0 );


	vec2 cc = vec2( 0.1 + sin( phase ) * .2, 0.61 - sin( phase ) * .2) * 1.0;
	
	vec3 dmin = vec3( 1000.0, 1000.0, 1000.0 );
	vec3 norm = vec3( 0.0, 0.0, 0.0 );
    vec2 z = scale * pRotated + vec2( -.415230, -.568869);
    for( int i=0; i<48; i++ )
    {
        z = cc + vec2( z.x*z.x - z.y*z.y, 2.0*z.x*z.y );
		z += 0.15*sin(float(i));
		norm.y = dot( z, z );
		norm.x = norm.y + 0.1 * z.y / ( z.x * z.x + .01) + sinuses * sin(1.0 * norm.y ) ;
		norm.z = 0.1 / length( fract( z * fractZScale ) - 0.5 );
		norm.z *= norm.z;
		
		//norm = .050 * ( prev + norm );
		
		
		dmin=min(dmin, norm );
    }
	float val = ( dmin.x - dmin.y + .83 );
								
	float inWeight = clamp( norm.z * 1.0, .0, 1.0 );
	vec3 colorIn = mix( vec3( 1.3, .984, .820 ), vec3( 1.8, .3, -.2 ), inWeight );
	
	float outWeight = clamp( 3.0 -5.0 *  norm.z, .0, 1.0 );
	vec3 colorOut = mix( vec3( 0.7, .2, .3 ), vec3( .173, .0, .137 ), outWeight );
				 
	
	float backgroundBlack = clamp(.3 * val * (vignetteVal - .5 ), .0, 1.0 );
	float backgroundWhite = clamp(-.3 * val, .0, 1.0 );
    val =  clamp( val * 3.0, .0, 1.0 );
    vec3 color = mix( vec3( 1.3, .984, .820 ), vec3( .173, .0, .137 ), val );
	
	color = mix( color, colorOut, backgroundBlack * 1.0);
	color = mix( color, colorIn, backgroundWhite * 1.0);
	
	
	color *= vignetteVal;
	
	
	gl_FragColor = vec4(color,1.0);
}
