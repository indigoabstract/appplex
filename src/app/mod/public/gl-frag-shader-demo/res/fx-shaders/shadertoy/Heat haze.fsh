// https://www.shadertoy.com/view/XdfXzf

// iChannel0: t4
// iChannel1: t5

#define distortFocus .3

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	float horizontDistance = abs(uv.y - distortFocus);
	float smallWaves = sin((uv.y+iGlobalTime*.01)*45.)*.06;
	float horizontalRipples = smallWaves / (horizontDistance*horizontDistance*1000. + 1.);

	
	float squiglyVerticalPoints = uv.x + smallWaves*.08;
	float verticalRipples = sin((squiglyVerticalPoints+iGlobalTime*.01)*60.) * .005;
	
	vec4 img = texture2D(iChannel0, vec2(uv.x + verticalRipples, -uv.y + horizontalRipples));
	gl_FragColor = img;
}
