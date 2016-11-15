// https://www.shadertoy.com/view/4d2Xzw

// iChannel0: t3
// iChannel1: t4

// Bokeh disc.
// by David Hoskins.
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

#define ITERATIONS 150.0

#define ONEOVER_ITR  1.0 / ITERATIONS
#define PI 3.141596

// This is (3.-sqrt(5.0))*PI radians, which doesn't precompiled for some reason.
// The compiler is a dunce I tells-ya!!
#define GOLDEN_ANGLE 2.39996323

//-------------------------------------------------------------------------------------------
// This creates the 2D offset for the next point.
// (r-1.0) is the same as sqrt(0, 1,  2, 3...)
vec2 Sample(in float theta, inout float r)
{
    r += 1.0 / r;
	return (r-1.0) * vec2(cos(theta), sin(theta)) * .06;
}

//-------------------------------------------------------------------------------------------
vec3 Bokeh(sampler2D tex, vec2 uv, float radius, float amount)
{
	vec3 acc = vec3(0.0);
	vec3 div = vec3(0.0);
    vec2 pixel = vec2(iResolution.y/iResolution.x, 1.0) * radius * .025;
    float r = 1.0;
	for (float j = 0.0; j < GOLDEN_ANGLE * ITERATIONS; j += GOLDEN_ANGLE)
    {
       	
		vec3 col = texture2D(tex, uv + pixel * Sample(j, r)).xyz;
        col = col * col * 1.2; // ...contrast it for better highlights
		vec3 bokeh = vec3(.5) + pow(col, vec3(10.0)) * amount;
		acc += col * bokeh;
		div += bokeh;
	}
	return acc / div;
}

//-------------------------------------------------------------------------------------------
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
    float time = iGlobalTime*.2 + .5;
	float r = .5 - .5*cos(time * PI * 2.0);
       
	float a = 120.0;
    if (iMouse.w >= 1.0)
    {
    	r = (iMouse.x/iResolution.x)*3.0;
        a = iMouse.y/iResolution.y * 80.0;
    }
    
    if (mod(time-.5, 2.0) < 1.0)
    {
		gl_FragColor = vec4(Bokeh(iChannel0, uv*vec2(1.0, -1.0), r, a), 1.0);
    }else
    {
        gl_FragColor = vec4(Bokeh(iChannel1, uv*vec2(1.0, -1.0), r, a), 1.0);    
    }
}
