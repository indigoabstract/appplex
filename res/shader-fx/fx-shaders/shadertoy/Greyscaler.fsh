// https://www.shadertoy.com/view/XdXSzX

// iChannel0: t4

// Various ways of converting to grey scale: http://blog.demofox.org/2014/02/03/converting-rgb-to-grayscale/
/*
  Written by Alan Wolfe
  http://demofox.org/
  http://blog.demofox.org/

  more info on this shader:
  http://blog.demofox.org/2014/02/03/converting-rgb-to-grayscale/
*/

void main(void)
{	
	vec2 percent = (gl_FragCoord.xy / iResolution.xy);
	percent.y = 1.0 - percent.y;
	
	vec3 pixelColor = texture2D(iChannel0, percent).xyz;	
	
	float mode = mod(iGlobalTime, 8.0) / 2.0;
	if (mode > 3.0)
	{
		// sRGB monitors grey scale coefficients
		float pixelGrey = dot(pixelColor, vec3(0.2126, 0.7152, 0.0722));
		pixelColor = vec3(pixelGrey);
	}
	else if (mode > 2.0)
	{
		// SD television grey scale coefficients
		float pixelGrey = dot(pixelColor, vec3(0.3, 0.59, 0.11));
		pixelColor = vec3(pixelGrey);
	}
	else if (mode > 1.0)
	{
		// naive grey scale conversion - average R,G and B
		float pixelGrey = dot(pixelColor, vec3(1.0/3.0));
		pixelColor = vec3(pixelGrey);
	}
	else
	{
		// color
	}
	
	gl_FragColor = vec4(pixelColor, 1.0);			
}
