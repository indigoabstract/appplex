// https://www.shadertoy.com/view/XssSzN
// Bounded metaballs with cubic falloff. It's usually recommended to use quintic falloffs though. See https://www.shadertoy.com/view/ld2GRz. And PLEASE don't use 1/d² potentials!
// Created by inigo quilez - iq/2014
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

void main( void )
{
	vec2 uv = (-iResolution.xy+2.0*gl_FragCoord.xy)/iResolution.y;

    // anim
	vec2 c1 = 0.8*sin( iGlobalTime*1.0 + vec2(4.0,0.5) + 1.0);
	vec2 c2 = 0.8*sin( iGlobalTime*1.3 + vec2(1.0,2.0) + 2.0);
	vec2 c3 = 0.8*sin( iGlobalTime*1.5 + vec2(0.0,2.0) + 4.0);
	
    // potential (3 metaballs)
    float v = 0.0;	
	v += 1.0-smoothstep(0.0,0.7,length(uv-c1));
	v += 1.0-smoothstep(0.0,0.7,length(uv-c2));
	v += 1.0-smoothstep(0.0,0.7,length(uv-c3));

    // color	
	vec3 col = mix( vec3(v), vec3(1.0,0.6,0.0), smoothstep(0.9,0.91,v) );
	
	gl_FragColor = vec4(col,1.0);
}
