// https://www.shadertoy.com/view/MsfGzM
// Created by inigo quilez - iq/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

float f(vec3 p) 
{ 
	p.z+=iGlobalTime;return length(.05*cos(9.*p.y*p.x)+cos(p)-.1*cos(9.*(p.z+.3*p.x-p.y)))-1.; 
}
void main()
{
    vec3 d=.5-gl_FragCoord.xyz/iResolution.x,o=d;for(int i=0;i<99;i++)o+=f(o)*d;
    gl_FragColor=vec4(abs(f(o-d)*vec3(.0,.1,.2)+f(o-.6)*vec3(.2,.12,.01))*(10.-o.z),1.);	
}
