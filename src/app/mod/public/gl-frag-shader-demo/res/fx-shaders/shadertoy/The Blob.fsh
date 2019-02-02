// https://www.shadertoy.com/view/XsXGRS
// by nikos papadopoulos, 4rknova / 2013
// Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

#ifdef GL_ES
precision highp float;
#endif

#define CI vec3(.3,.5,.6)
#define CO vec3(.2)
#define CM vec3(.0)
#define CE vec3(.8,.7,.5)

float metaball(vec2 p, float r)
{
	return r / dot(p, p);
}

void main(void)
{
	vec2 uv = (gl_FragCoord.xy / iResolution.xy * 2. - 1.)
			* vec2(iResolution.x / iResolution.y, 1);
	
	float t0 = sin(iGlobalTime * 1.9) * .46;
	float t1 = sin(iGlobalTime * 2.4) * .49;
	float t2 = cos(iGlobalTime * 1.4) * .57;

	float r = metaball(uv + vec2(t0, t2), .33) *
			  metaball(uv - vec2(t0, t1), .27) *
			  metaball(uv + vec2(t1, t2), .59);

	vec3 c = (r > .4 && r < .7)
			  ? (vec3(step(.1, r*r*r)) * CE)
			  : (r < .9 ? (r < .7 ? CO: CM) : CI);

	gl_FragColor = vec4(clamp(c, 0., 1.), 1);
}
