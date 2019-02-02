// https://www.shadertoy.com/view/XsX3Rn

// iChannel0: t0

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

void main(void)
{
    vec2 p = -1.0+2.0*gl_FragCoord.xy/iResolution.y;
    
    float an = iGlobalTime*0.1;
    float x = p.x*cos(an)-p.y*sin(an);
    float y = p.x*sin(an)+p.y*cos(an);
     
    vec2 uv = 0.2*vec2(x,1.0)/abs(y);
    uv.xy += 0.20*iGlobalTime;
	
	float w = max(-0.1, 0.6-abs(y) );
	gl_FragColor = vec4( texture2D(iChannel0, uv).xyz+w, 1.0);
}
