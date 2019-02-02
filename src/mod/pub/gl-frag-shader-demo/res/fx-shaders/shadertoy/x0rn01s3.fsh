// https://www.shadertoy.com/view/4ssSzf
// vincent francois - cyanux/2014
// xor noise

#define BITS 8.0

float xor(vec2 p)
{
	float ret = 0.0;

	for(float i = BITS; i > 1.0; i--)
	{
		float j = exp2(i);

		if((j < p.x)  ^^ (j < p.y))
			ret += j;

		if(j < p.x)
			p.x -= j;

		if(j < p.y)
			p.y -= j;
	}

	return ret;
}

float noise(vec2 p, float z)
{
	return fract(xor(p) / z);
}

// hsv2rgb - iq / https://www.shadertoy.com/view/MsS3Wc
vec3 hsv2rgb( in vec3 c ) {
    vec3 rgb = clamp( abs(mod(c.x*6.0+vec3(0.0,4.0,2.0),6.0)-3.0)-1.0, 0.0, 1.0 );
	return c.z * mix( vec3(1.0), rgb, c.y);
}

void main()
{
	vec2 uv = (gl_FragCoord.xy / iResolution.xy) * 512.0;
	
	float h = (sin(2.0 * iGlobalTime + gl_FragCoord.y / iResolution.y) + 1.0) * 0.2 + 0.2;
	float v = noise(uv, ceil(mod(iGlobalTime * 32.0, 512.0)));
	gl_FragColor.rgb = hsv2rgb(vec3(h, 1.0, v));
}
