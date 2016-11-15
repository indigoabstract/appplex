// https://www.shadertoy.com/view/4sS3Dc
// I needed to investigate and understand the HSL colorspace for a computer-vision image filter and I thought to myself 'How can I visualize this quickly?' and then this shader just sort of happened. :)
// Created by Per Bloksgaard/2014

#define PI 3.14159265358979

// Convert HSL colorspace to RGB. http://en.wikipedia.org/wiki/HSL_and_HSV
vec3 HSLtoRGB(in float h, in float s, in float l)
{
	vec3 rgb = clamp( abs(mod(h+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
	return l + s * (rgb-0.5)*(1.0-abs(2.0*l-1.0));
}

void main(void)
{
	vec2 r = gl_FragCoord.xy/iResolution.xy;
	vec2 p = -1.0+2.0*r;
	p.x *= iResolution.x/iResolution.y;
	float fSin = sin(iGlobalTime*0.4);
	float fCos = cos(iGlobalTime*0.4);
	p *= mat2(fCos,-fSin,fSin,fCos);
	float h = atan(p.x,p.y)+PI;
	float x = distance(p,vec2(0.0));
	float a = -(0.6+0.2*sin(iGlobalTime*3.1+sin((iGlobalTime*0.8+h*2.0)*3.0))*sin(iGlobalTime+h));
	float b = -(0.8+0.3*sin(iGlobalTime*1.7+sin((iGlobalTime+h*4.0))));
	float c = 1.25+sin((iGlobalTime+sin((iGlobalTime+h)*3.0))*1.3)*0.15;
	float l = a*x*x + b*x + c;
	gl_FragColor = vec4(HSLtoRGB(h*3.0/PI,1.0,l),1.0);
}
