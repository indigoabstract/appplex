// https://www.shadertoy.com/view/4dXSzl

// iChannel0: t4
// iChannel1: t13

#define pixelSize 2.

#define colorDetail 1.

#define _ 0.0
#define o (1./3.)
#define b (2./3.)
#define B 1.0

#define check(r,g,b) color=vec4(r,g,b,0.); dist = distance(sample,color); if (dist < bestDistance) {bestDistance = dist; bestColor = color;}

void main(void)
{
	float dist;
	float bestDistance = 1000.;
	vec4 color;
	vec4 bestColor;		
	
	vec2 pixel = floor( gl_FragCoord.xy / pixelSize);
	vec4 sample = texture2D(iChannel0, pixel * pixelSize / iResolution.xy);
	
	sample += (texture2D(iChannel1, pixel / iChannelResolution[1].xy)-0.5)*0.5;
	
	sample = floor(sample*colorDetail+0.5)/colorDetail;	
	
	check(_,_,_);
	check(_,_,b);
	check(_,b,_);
	check(_,b,b);
	check(b,_,_);
	check(b,_,b);
	check(b,o,_);
	check(b,b,b);
	
	check(o,o,o);
	check(o,o,B);
	check(o,B,o);
	check(o,B,B);
	check(B,o,o);
	check(B,o,B);
	check(B,B,o);
	check(B,B,B);	
	
	gl_FragColor = bestColor;	
}
