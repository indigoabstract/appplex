// https://www.shadertoy.com/view/XdfSR7
// Generates a circle & gradient for HSV (hue, saturation, value) colour picking in your applications
vec3 hsv2rgb(vec3 c) // http://stackoverflow.com/a/17897228/605869
{
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.yy;
	uv = uv*2.0-vec2(1,1);
	float a = atan(uv.x,uv.y);
	a =(a / 3.14159*0.5)+1.0 ;
	float d = length(uv);
	vec3 c = hsv2rgb(vec3(a,d,1.0));
	if(uv.x > 1.5) // remove this if you don't want the 'V' component gradient
	{
		float b = (uv.y+1.0)/2.0;
		gl_FragColor = vec4(b,b,b,1.0);
	}
	else if(d > 1.0)
	{
		discard;
	}
	else
	{
		gl_FragColor = vec4(c,1.0);
	}
}
