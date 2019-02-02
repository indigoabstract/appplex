// https://www.shadertoy.com/view/ldBGRW
void main(void)
{
	vec2 vv = gl_FragCoord.xy; // + vec2( 0.5, 0.5 );
	vv.x += sin( iGlobalTime * 2.0 ) * 40.0;
	vv.y += cos( iGlobalTime * 2.3 ) * 30.0;
	
	vv -= iResolution.xy / 2.0;
	vec4 color = vec4( .0, .7, .2, 1.0 );
	if ( sin( atan( vv.y, vv.x ) * 10.0 + iGlobalTime * 10.0 + length(vv) / 10.0 ) > 0.0 )
		color = vec4( .3, .0, .4, 1.0 );
	color.xyz *= length(vv) / (length(iResolution.xy)/3.0);
	gl_FragColor = color;
}
