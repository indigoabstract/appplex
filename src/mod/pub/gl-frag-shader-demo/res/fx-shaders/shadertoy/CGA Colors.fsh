// https://www.shadertoy.com/view/4dXXzl

// iChannel0: t4
// iChannel1: t13

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
	
	vec2 pixel = floor( gl_FragCoord.xy / iResolution.xy * vec2(320,200));
	vec4 sample = texture2D(iChannel0, pixel / vec2(320,200));
	
	sample += (texture2D(iChannel1, pixel / iChannelResolution[1].xy)-0.5)*0.5;
	
	// pallete 0
	
	check(_,_,_);
	check(o,B,o);
	check(B,o,o);
	check(B,B,o);
	
	vec4 color1 = bestColor;
	bestDistance = 1000.;
	
	// pallete 1
	
	check(_,_,_);
	check(o,B,B);
	check(B,o,B);
	check(B,B,B);
	
	vec4 color2 = bestColor;
	
	float t = (clamp(sin(iGlobalTime)*4.,-1.,1.)+1.)/2.;
	
	gl_FragColor = color1 * t + color2 * (1.-t);	
}
