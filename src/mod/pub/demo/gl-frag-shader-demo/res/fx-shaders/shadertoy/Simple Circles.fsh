// https://www.shadertoy.com/view/4djGDc
/*
  Andor Salga
  March 2014
*/

void main(void)
{
	vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 2. - 1.;
	vec2 aspectRatio = vec2(iResolution.x / iResolution.y, 1.);
	float time = -iGlobalTime * 5.;
	gl_FragColor = vec4( sin(length(uv * aspectRatio * .8) * 25. + time) );
}
