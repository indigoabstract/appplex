// https://www.shadertoy.com/view/ldsGzn
// Copyright (c) 2013 Andrew Baldwin (baldand)
// License = Attribution-NonCommercial-ShareAlike (http://creativecommons.org/licenses/by-nc-sa/3.0/deed.en_US)

// "Cube bird"
// A colourful accident that appeared while I was making and debugging a simple distance field ray marcher

const vec3 screen = vec3(0.);
const vec3 up = vec3(0.,1.,0.);
const vec3 boxPos = vec3(0.,0.,2.);

float box(vec3 p,vec3 b)
{
  return length(max(abs(p)-b,0.0));
}

float world(vec3 p)
{
	return box(p-boxPos,vec3(0.3));
}

void main(void)
{
	vec3 eye = vec3(1.*sin(iGlobalTime),.5*cos(iGlobalTime),-3.);
    vec2 screenSize = vec2(iResolution.x/iResolution.y,1.0);
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 offset = screenSize * (uv - 0.5);
	vec3 right = cross(up,normalize(screen - eye));
	vec3 ro = screen + offset.y*up + offset.x*right;
	vec3 rd = normalize(ro - eye);
	float d=0.0;
	vec3 r = ro;
	for (int i=0;i<10;i++) {
		d = world(r);
		if (d<0.0) return;
		r += d*rd;
	}
	gl_FragColor = vec4(vec3(2.*abs(r-boxPos)),1.0);
}
