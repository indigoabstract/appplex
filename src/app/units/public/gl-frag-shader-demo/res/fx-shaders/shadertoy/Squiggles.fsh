// https://www.shadertoy.com/view/4sjXRh
// Squiggles
// Dave H.

// https://www.shadertoy.com/view/4sjXRh

//---------------------------------------------------------------------------------------
vec2 Hash22(vec2 p)
{
	p  = fract(p * vec2(5.3983, 5.4427));
    p += dot(p.yx, p.xy +  vec2(21.5351, 14.3137));
	return fract(vec2(p.x * p.y * 95.4337, p.x * p.y * 97.597));
}

//---------------------------------------------------------------------------------------
vec2 Noise( in vec2 x )
{
    return mix(Hash22(floor(x)), Hash22(floor(x)+1.0), fract(x));
}

//---------------------------------------------------------------------------------------
vec2 HashMove2(vec2 p)
{
    return Noise(p+iGlobalTime*.1);
}

//---------------------------------------------------------------------------------------
vec3 Cells(in vec2 p, in float time)
{
    vec2 f = fract(p);
    p = floor(p);
	float d = 1.0e10;
    vec2 id = vec2(0.0);
    
	for (int xo = -1; xo <= 1; xo++)
	{
		for (int yo = -1; yo <= 1; yo++)
		{
            vec2 g = vec2(xo, yo);
            vec2 n = HashMove2(p + g);
            n = n*n*(3.0-2.0*n);
            
			vec2 tp = g + .5 + sin(time * 1.5 + 6.2831 * n)*1.2 - f;
            float d2 = dot(tp, tp);
			if (d2 < d)
            {
                // 'id' is the colour code for each squiggle
                d = d2;
                id = n;
            }
		}
	}
	return vec3(sqrt(d), id);
}

//---------------------------------------------------------------------------------------
void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xx;
	float time = iGlobalTime;
    vec3 col = vec3(0.0);
	float amp = 1.0;
    float size = 4.0 * (abs(fract(time*.01-.5)-.5)*50.0+1.0) + ((iMouse.x/iResolution.x) * 200.0);
    float timeSlide = sin(time*.24)*.02 + .03;
         
    for (int i = 0; i < 20; i++)
    {
        vec3 res = Cells(uv * size - size * .5, time);
        float c = 1.0 - res.x;
        // Get a colour associated with the returned id...
        vec3 wormCol =  clamp(abs(fract((res.y+res.z)* 1.1 + vec3(1.0, 2.0 / 3.0, 1.0 / 3.0)) * 6.0 - 3.0) -1.0, 0.0, 1.0);
        c = smoothstep(0.6+amp*.25, 1., c);
        col += amp * c * ((wormCol * .1) + vec3(.9, .2, .15));
        amp *= .85;
        time -= timeSlide;
    }
	gl_FragData[0] = vec4(min(col, 1.0), 1.0);
}
