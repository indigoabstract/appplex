// https://www.shadertoy.com/view/XdfGzn

// iChannel0: t8

// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

void main(void)
{
    vec2 p = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
    p.x *= iResolution.x/iResolution.y;

    vec2 cst = vec2( cos(iGlobalTime), sin(iGlobalTime) );
    mat2 rot = (1.0 + 0.5*cst.x)*mat2(cst.x,-cst.y,cst.y,cst.x);

    vec3 col = texture2D( iChannel0, fract(0.5+0.5*rot*p) ).xyz;
    gl_FragColor = vec4(col,1.0);
}
