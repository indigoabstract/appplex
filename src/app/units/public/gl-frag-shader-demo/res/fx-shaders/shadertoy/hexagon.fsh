// https://www.shadertoy.com/view/4ssGz7
#define PI 3.14159265359

float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

float gap(int t) {
	return floor(rand(vec2(float(t))) * 5.9999);
}

bool occupied(int t, float i) {
	return rand(vec2(i, float(t))) < 5.0 / 6.0 && gap(t) != i;
}

vec3 hsv_to_rgb(float h, float s, float v)
{
	float c = v * s;
	h = mod((h * 6.0), 6.0);
	float x = c * (1.0 - abs(mod(h, 2.0) - 1.0));
	vec3 color;
 
	if (0.0 <= h && h < 1.0) {
		color = vec3(c, x, 0.0);
	} else if (1.0 <= h && h < 2.0) {
		color = vec3(x, c, 0.0);
	} else if (2.0 <= h && h < 3.0) {
		color = vec3(0.0, c, x);
	} else if (3.0 <= h && h < 4.0) {
		color = vec3(0.0, x, c);
	} else if (4.0 <= h && h < 5.0) {
		color = vec3(x, 0.0, c);
	} else if (5.0 <= h && h < 6.0) {
		color = vec3(c, 0.0, x);
	} else {
		color = vec3(0.0, 0.0, 0.0);
	}
 
	color += v - c;
 
	return color;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	uv -= 0.5;
	
	uv.x /= iResolution.y / iResolution.x;
	
	float time = iGlobalTime * 1.3;
	int t = int(time);
	float s = fract(time);
	float ao = time;
	
	float a = mod(atan(uv.y, uv.x) + PI + ao, PI * 2.0);
	float i = floor(a / 2.0 / PI * 6.0);
	float da = a - (i + 0.5) * 2.0 * PI / 6.0;
	float d = length(uv) * cos(da) - length(texture2D(iChannel0, vec2(0.0, 0.0))) * 0.03;
	
	float brightness;
	if (d < 0.015) {
		brightness = 0.05;
	} else if (abs(d - 0.015) < 0.003) {
		brightness = 0.9;
	} else {
		bool l = occupied(t, i);
		
		if (l && abs(d - 0.9 + s) < 0.05) {
			brightness = 0.9;
		} else {
			float target = mix(gap(t - 1), gap(t), clamp(s * 2.0 - 0.4, 0.0, 1.0)) - 1.0;
			float dir = target / 6.0 * 2.0 * PI - ao;
			
			vec2 x = vec2(cos(dir), sin(dir));
			vec2 y = vec2(-x.y, x.x);
			
			vec2 uv2 = vec2(dot(uv, x), dot(uv, y));
			
			if (abs(uv2.x) < uv2.y + 0.1 && uv2.y < -0.087) {
				brightness = 0.9;
			} else {
				brightness = 0.05 + floor(mod(i + iGlobalTime, 2.0)) * 0.05;
			}
		}
	}
	
	vec3 bg = hsv_to_rgb(iGlobalTime * 0.05, 1.0, 1.0);
	gl_FragColor = vec4(bg * brightness, 1.0);
}
