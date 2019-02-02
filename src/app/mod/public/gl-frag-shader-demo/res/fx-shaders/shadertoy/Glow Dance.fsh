// https://www.shadertoy.com/view/ldjXWD
#define GLOW 1.0

float	dSphere(vec2 pCenter, float pRayon)
{
    return length(gl_FragCoord.xy - pCenter) - pRayon;
}

vec3	sSphereColor(vec3 pColor, float ld)
{
    return pColor * (1.0 - smoothstep(-2.0, 0.0, ld));
}

vec3	sGlow(vec3 pColor, float ld)
{
    return pColor * exp(-(ld/50.0)*0.5) * GLOW;
}

void main(void)
{
    float	lt1 = sin(iGlobalTime);
    float	lt2 = cos(iGlobalTime);
    float	ld1 = dSphere(vec2(iResolution.xy*0.5) - vec2(50.0*lt1, 200.0*lt2), 25.0);
    float	ld2 = dSphere(vec2(iResolution.xy*0.5), 25.0);
    float	ld3 = dSphere(vec2(iResolution.xy*0.5) + vec2(200.0*lt1, 50.0*lt2), 25.0);
    vec3	lColor = sSphereColor(vec3(1.0, 0.0, 0.0), ld1) +
        			 sSphereColor(vec3(0.0, 1.0, 0.0), ld2) +
        			 sSphereColor(vec3(0.0, 0.0, 1.0), ld3);
    lColor += sGlow(vec3(1.0, 0.0, 0.0), ld1);
    lColor += sGlow(vec3(0.0, 1.0, 0.0), ld2);
    lColor += sGlow(vec3(0.0, 0.0, 1.0), ld3);
    gl_FragColor = vec4(lColor, 1.0);
}
