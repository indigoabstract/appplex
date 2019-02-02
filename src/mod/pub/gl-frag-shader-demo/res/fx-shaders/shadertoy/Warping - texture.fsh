// https://www.shadertoy.com/view/Xsl3zn

// iChannel0: t8

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

void main(void)
{
	vec2 uv = 0.5*gl_FragCoord.xy / iResolution.xy;

	float d = length(uv);
	vec2 st = uv*0.1 + 0.2*vec2(cos(0.071*iGlobalTime+d),
								sin(0.073*iGlobalTime-d));

    vec3 col = texture2D( iChannel0, st ).xyz;
    float w = col.x;
	col *= 1.0 - texture2D( iChannel0, 0.4*uv + 0.1*col.xy  ).xyy;
	col *= w*2.0;
	
	col *= 1.0 + 2.0*d;
	gl_FragColor = vec4(col,1.0);
}
