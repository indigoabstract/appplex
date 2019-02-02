// https://www.shadertoy.com/view/MdS3Dc
/*---------------------------------------------------------------------------
Glitch07 by musk/brainstorm                 Tokyo Demo Fest 2014 2014/3/21-23
                                                  7 Lines GLSL Graphics Compo

Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

features:

	animation:
	 - time based intro effect (first 10 seconds)
	 - rotating object
	geometry:
	 - raymarching
	 - geometry repetition via rotation
	 - signed distance field is a simple box
	shading:
	 - one sample ambient occlusion
	post processing:
	 - gamma correction
	 - vignette

tested on: 
	
	NVIDIA 9800, AMD A4 and I get the same result. This means that the 
shader is stable even though it looks like a random glitch :D

How to run it?
	1. Open http://glsl.heroku.com/e in your browser
	2. Copy the text from this file
	3. Paste it into the code editor on the website
	PS: your browser needs WebGL

Obligatory section which doesn't count into the 7 lines :D                 */
#ifdef GL_ES
precision mediump float;
#endif
                                                                           /*
Here are the 7 lines:
----8<---------------------------------------------------------------------*/
float t=iGlobalTime*.5;float m(vec3 p){p=abs(p);return max(p.x-6.5,max(p.y-.4
,p.z-.4));}vec2 r(vec2 p){float a=mod(atan(p.x,p.y),.4)-.5;return vec2(cos(a)
,sin(a))*length(p);}float f(vec3 p){p.xz*=mat2(sin(t),cos(t),-cos(t),sin(t));
p.xz=r(p.xz);p.yz=r(p.yz);p.xy=r(p.xy);return (m(p));}void main(void){vec2 uv
=gl_FragCoord.xy/iResolution.yy-vec2(.9,.5); vec3 p=vec3(cos(uv*999.0)/(t*t*t
),-5.),d=vec3(uv,.5);for(int i=0;i<50;i++){p+=d*f(p);}gl_FragColor = vec4(min
(pow(f(p-d),.5),1.)-length(uv)*.5);}
/*--8<---------------------------------------------------------------------*/

//glsl.heroku.com version (compo version)
/*--8<-----------------------------------------------------------------------
uniform float time;uniform vec2 resolution;float t=time*.5;float m(vec3 p){p=
abs(p);return max(p.x-6.5,max(p.y-.4,p.z-.4));}vec2 r(vec2 p){float a=mod(
atan(p.x,p.y),.4)-.5;return vec2(cos(a),sin(a))*length(p);}float f(vec3 p){p.
xz*=mat2(sin(t),cos(t),-cos(t),sin(t));p.xz=r(p.xz);p.yz=r(p.yz);p.xy=r(p.xy)
;return (m(p));}void main(void){vec2 uv=gl_FragCoord.xy/resolution.yy-vec2(.9
,.5); vec3 p=vec3(cos(uv*999.0)/(t*t*t),-5.),d=vec3(uv,.5);for(int i=0;i<50;i
++){p+=d*f(p);}gl_FragColor = vec4(min(pow(f(p-d),.5),1.)-length(uv)*.5);}
----8<---------------------------------------------------------------------*/
