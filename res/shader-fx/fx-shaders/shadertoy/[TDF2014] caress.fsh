// https://www.shadertoy.com/view/Md2GDt
// specially for 7x79 GLSL compo at Tokyo Demo Fest 2014
// to-shadertoy preamble
float time=iGlobalTime;vec2 resolution=iResolution.xy;float N(vec2 v){return fract(sin(dot

//orig: uniform float time;uniform vec2 resolution;float N(vec2 v){return fract(sin(dot
(vec2(7.,23.),v))*519.);}vec2 e=vec2(1.,0.),s,f,F;float M(vec2 v){F=floor(v);f=
fract(v);f*=f*(3.-2.*f);return mix(mix(N(F),N(F+e.xy),f.x),mix(N(F+e.yx),N(F+e.
xx),f.x),f.y);}float B(vec2 v){return M(v)+.5*M(v*2.)+.2*M(v*8.);}float t=time,
l=0.,r;void main(){for(int i=0;i<99;++i){vec3 q=vec3(gl_FragCoord.xy/resolution
-.5,1.)*l;q.z-=2.;q.x*=1.8;r=length(q)-1.;s=42.*(q.xy+M(vec2(r-t*.5))-M(vec2(r-
t*.3))*e.xy);l+=.4*(r+.2*B(s));}gl_FragColor=1.-vec4(B(s),B(s+.1),B(s+.3),1.);}

// I DEMAND AN EXPLANATION
//
// TL;DR: not much, just some common techinques squeezed together.
//
// let's break down the lines a bit:
//
// line 1:
// obvious and uninteresting uniform declarations
// 	uniform float time;uniform vec2 resolution;
//
// lines 1-2:
// hash noise function R^2 -> R
// 	float N(vec2 v) { 
//		return fract(sin(
//
// get from vec2 to float
//			dot(vec2(7.,23.),v)
//		)*519.);
//	}
//
// line 2:
// declare all vec2 vars in one place so there as little declaration overhead as possible
// 	vec2 e=vec2(1.,0.),s,f,F;
// 
// lines 2-4:
// C2-smooth value noise from hash noise.
// simply a bilinear interpolation of 4 adjacent hashes
// 	float M(vec2 v){
//
// cell params
// F = bottom-left vertex coordinates
// f = sampling point coords relative to F
// 		F=floor(v);
//		f=fract(v);
//
// guarantee smoothness:
//		f*=f*(3.-2.*f);
//
// interpolate bilinearly
//		return mix(
//			mix(N(F),N(F+e.xy),f.x),
//			mix(N(F+e.yx),N(F+e.xx),f.x),
//			f.y);
//	}
//
// line 4:
// fractal value noise sum with 3 octaves
//	float B(vec2 v){
//		return
//			M(v)
//			+.5*M(v*2.)
//			+.2*M(v*8.);
//	}
//
// lines 4-5:
// as before with vec2 declare all float variables in one place
// float t=time,l=0.,r;
// l is raymarched distance
//
// lines 5-7: void main
// simple sdf raymarching
// 	void main(){
//
// for max 99 ray samples
//		for(int i=0;i<99;++i){
//
// calculate ray sample position from screen pixel coords and marched distance l
//			vec3 q=vec3(gl_FragCoord.xy/resolution-.5,1.)*l;
//
// begin ray at (0., 0., -2.)
//			q.z-=2.;
//
// fix aspect at ~16:9
//			q.x*=1.8;
//
// get distance to sphere center
// 			r=length(q)-1.;
//
// calculate displacement map coordinates
// 			s=42.*(
//
// begin with current xy screen coordinates
//				q.xy
//
// add random "bottom-left to top-right" rotation
// note that we have only a vec2 noise function so we need to wrap arguments in vec2.
// this is cheaper than having separate float N(float) by a few bytes.
// subtracting time from radius gives the wavy fur look
//				+M(vec2(r-t*.5))
//
// add analogous random "right to left" rotation just to add one more visible degree of freedom
//				-M(vec2(r-t*.3))*e.xy);
//
// sample radius displacement as fractal noise and step along the ray accordingly
//			l+=.4*(r+.2*B(s));
//		}
//
// re-use displacement coordinates as a hash value for determining color
//		gl_FragColor=1.-vec4(B(s),B(s+.1),B(s+.3),1.);
//	}
