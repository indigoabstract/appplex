// https://www.shadertoy.com/view/ldlXz7

// iChannel0: t0

// For good scaling, X / Y = 4
#define TEX_REPEAT_X 4.
#define TEX_REPEAT_Y 1.

#define TEX_WIDTH 1.

// on top of my head
#define PI 3.141592

vec2 frustum = vec2(iResolution.x / iResolution.y, 1.) / iResolution.xy;

// I don't even need to explain anything, do I ? If so, don't hesitate to ask in comments.

void main(void)
{
	vec2 uv = gl_FragCoord.xy * frustum * 2. - 1.;
	uv += iMouse.xy * frustum * -2. + 1.;
	
	vec2 texel = vec2(atan(uv.x, uv.y) * (TEX_WIDTH / PI) * TEX_REPEAT_X / 2.,
					  TEX_REPEAT_Y / distance(uv, vec2(0.)));
	
	vec2 offset = vec2(cos(iGlobalTime), iGlobalTime);
	float shade = clamp(0., 1., distance(uv, vec2(0)));
	gl_FragColor = texture2D(iChannel0, texel + offset) * shade;
}
