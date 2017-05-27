// http://glslsandbox.com/e#21097.4
#ifdef GL_ES
precision mediump float;
#endif 

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;
uniform sampler2D bb;

const float ir = 1.;
const float or = 10.0;

//#define LOOP 

float pi = 3.14159;

vec4 get(vec2 p)
{
	#ifdef LOOP
	return texture2D(bb,mod(p/resolution,1.));	
	#else
	return texture2D(bb,p/resolution);
	#endif
}

float weight(vec2 p,float l)
{
	return ((1.-sin(l*0.85))*exp(-l*0.34)*(1./.235)/.125);		
}

float kernel(vec2 p)
{
	float w = 0.0;
	float tw = 0.0;
	for(float y = -or;y <= or;y++)
	{
		for(float x = -or;x <= or;x++)
		{
			float tmpw = weight(p+vec2(x,y),length(vec2(x,y)));
			w += get(p+vec2(x,y)).w*tmpw;
			tw += tmpw;
		}
	}
	return w/tw;
}

float randb(vec2 co){
    return (fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453) > 0.75) ? 1.0 : 0.0;
}

void main( void ) {

	vec2 p = ( gl_FragCoord.xy );

	float c = get(p).w;
	float k = kernel(p);
	
	if(k > 0.236 && k < 0.31)
	{
		c += k;
	}
	else
	{
		c -= k;
	}
	
	/*
	if(length(mouse*resolution-p) < 16. )
	{
		c = 1.-randb(p+time);
	}
	if(length(mouse) < 0.02 || time < 0.2)
	{
		c = 0.0;
		if(length(p-resolution/2.) < 256.)
		{
			c = randb(p+time);
		}
		
	}
	*/
	c += pow(2.0*sin(time*.1), 4.)/(p.x+p.y);
	k += c;
	gl_FragColor = vec4( vec3( c*8. , c*2., k*.25), c );

}
