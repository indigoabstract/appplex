// https://www.shadertoy.com/view/4dB3zw
float rand(vec2 co)
{
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec4 HSVtoRGB(vec3 hsv)
{
	float i = floor(mod(floor(hsv.x / 60.0),6.0));
	float f = mod(hsv.x / 60.0, 6.0) - i;
	float p = hsv.z * (1.0 - hsv.y);
	float q = hsv.z * (1.0 - f * hsv.y);
	float t = hsv.z * (1.0 - (1.0 - f) * hsv.y);
	
	if (i == 0.) return vec4(hsv.z,t,p,1);
	if (i == 1.) return vec4(q,hsv.z,p,1);
	if (i == 2.) return vec4(p,hsv.z,t,1);
	if (i == 3.) return vec4(p,q,hsv.z,1);
	if (i == 4.) return vec4(t,p,hsv.z,1);
	return vec4(hsv.z,p,q,1);
}

vec4 pixel(vec2 p, vec2 c)
{
	for(int iter = 0; iter < 64; iter++) {
		if (length(p) > 4.0 + sin(iGlobalTime) * 2.0)
			return mix(
				HSVtoRGB(vec3(iGlobalTime * 34.0,1,0.5)),
				HSVtoRGB(vec3(iGlobalTime * 4.3,1,1)),
				float(iter) / 16.0);
		
		p = vec2(p.x * p.x - p.y * p.y, p.y * p.x + p.x * p.y) + c;
	}
	
	return HSVtoRGB(vec3(iGlobalTime * 4.3,1,1));
}

vec2 getPos(vec2 p)
{
	vec2 r = p / iResolution.xy;
	r -= vec2(0.5,0.5);
	r *= 2.0;
	r.x *= iResolution.x / iResolution.y;
	return r;
}

void main(void)
{
	if (iMouse.x == 0.0 && iMouse.y == 0.0) {
		float time = floor(iGlobalTime / 4.0);
		
		float x1 = rand(vec2(time,2.74)) * 3.0 - 2.0;
		float y1 = rand(vec2(time,3.71)) * 2.0 - 1.0;
		float x2 = rand(vec2(time + 1.0,2.74)) * 3.0 - 2.0;
		float y2 = rand(vec2(time + 1.0,3.71)) * 2.0 - 1.0;
		
		vec2 pos = mix(
			vec2(x1,y1),
			vec2(x2,y2),
			cos(mod(iGlobalTime, 4.0) / 4.0 * 3.14159) * (-0.5) + 0.5);
		gl_FragColor = pixel(getPos(gl_FragCoord.xy), pos);
	} else
		gl_FragColor = pixel(getPos(gl_FragCoord.xy), getPos(iMouse.xy));
}
