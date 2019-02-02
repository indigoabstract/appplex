// https://www.shadertoy.com/view/MdlXDM

// iChannel0: t7

// Photorealistic shaders involves the study of light, light involves photoreceptors. So after reading an article, I found an interesting effect... watch the hexagon for a while and you see green lights!
// vincent francois/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
// Watch the hexagon for a while and you see green lights!
//
// see http://www.photo-lovers.org/fcolor.html.fr

#define LIGHT_COLOR vec4(1.0, 0.0, 1.0, 0.0)
#define LIGHT_DISTANCE 0.4
#define LIGHT_QUANTITY 8.0
#define LIGHT_RADIUS 0.15

#define HEXAGON_WIDTH 0.01

const vec2 I = vec2(LIGHT_DISTANCE, 0.0);

vec2 rotate(vec2 v, float t)
{
	return vec2(v.x*cos(t)-v.y*sin(t),v.x*sin(t)+v.y*cos(t));
}

void draw_light(vec2 light, vec2 uv)
{
	float d = length(uv - light);
	if(min(LIGHT_RADIUS - d, 0.0) == 0.0)
		gl_FragColor = max((1.0 - (1.0 / LIGHT_RADIUS) * d) * LIGHT_COLOR, vec4(0.25));
}

#define LIGHT_ANGLE(INDEX) (INDEX * 6.28 / LIGHT_QUANTITY)

void main(void)
{
	gl_FragColor = vec4(0.25);
	
	vec2 ar = vec2(iResolution.x/iResolution.y, 1.0);
	vec2 uv = ar * (gl_FragCoord.xy / iResolution.xy - 0.5);
	
	if(max(abs(uv.x)-HEXAGON_WIDTH,0.0)+max(abs(uv.y)-HEXAGON_WIDTH,0.0)<HEXAGON_WIDTH)
	{
		gl_FragColor = vec4(1.0);
		return;
	}
	
	for(float i = 0.0; i < LIGHT_QUANTITY; i++)	{
		if(mod(ceil(iGlobalTime * 16.0), LIGHT_QUANTITY) != i)
			draw_light(rotate(I, LIGHT_ANGLE(i)), uv);
	}
}
