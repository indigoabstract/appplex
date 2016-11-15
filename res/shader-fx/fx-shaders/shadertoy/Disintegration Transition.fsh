// https://www.shadertoy.com/view/lslSz7#

// iChannel0: t4
// iChannel1: t0

float progress;
float randomSeed;

float LinearTween(float t, float start, float end)
{
	return t * start + (1. - t) * end;
}

float quadraticEaseIn(float t, float start, float end)
{
	return LinearTween(t * t, start, end);
}

float quadraticEaseInOut(float t, float start, float end)
{
	float middle = (start + end)/2.;
	t = 2. * t;
	if( t <= 1.)
		return LinearTween( t * t, start, middle);
	t -= 1.;
	return LinearTween(t * t, middle, end);
}
	
vec4 RadialBlur(vec2 uv)
{
    vec2 center = vec2(0.5,0.5);
    vec2 toUV = uv - center;
    vec2 normToUV = toUV;
    
    
    vec4 c1 = vec4(0.);
    int count = 24;
    float s = progress * 0.02;
    
    for(int i=0; i<24; i++)
    {
        c1 += texture2D(iChannel0, uv - normToUV * s * float(i)); 
    }
    
    c1 /= 24.;
    vec4 c2 = texture2D(iChannel1, uv);

    return mix(c1, c2, progress);
}

vec4 RadialWiggle(vec2 uv)
{
    vec2 center = vec2(0.5,0.5);
    vec2 toUV = uv - center;
    float distanceFromCenter = length(toUV);
    vec2  normToUV = toUV / distanceFromCenter;
    float angle = (atan(normToUV.y, normToUV.x) + 3.141592) / (0.2 * 3.141592);
    float offset1 = texture2D(iChannel2, vec2(angle, fract(progress/3. + distanceFromCenter/5. + randomSeed))).x * 2.0 - 1.0;
    float offset2 = offset1 * 2.0 * min(0.3, (1.-progress)) * distanceFromCenter;
    offset1 = offset1 * 2.0 * min(0.3, progress) * distanceFromCenter;
    
    vec4 c1 = texture2D(iChannel1, fract(center + normToUV * (distanceFromCenter + offset1))); 
    vec4 c2 = texture2D(iChannel0, fract(center + normToUV * (distanceFromCenter + offset2)));

    return mix(c1, c2, progress);
}

void main(void)
{
  randomSeed = 3.14159 ;
  progress	 = clamp(fract(iGlobalTime/1.5),0.0,1.0);
  progress	 = quadraticEaseIn(progress, 0.0, 1.0);
  // progress  *= progress;
	   
  vec2 p = gl_FragCoord.xy / iResolution.xy;
  gl_FragColor = RadialWiggle(p);
}
