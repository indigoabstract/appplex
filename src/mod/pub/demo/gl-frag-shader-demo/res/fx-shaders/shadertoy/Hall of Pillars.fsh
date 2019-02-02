// https://www.shadertoy.com/view/4dlXW4

// iChannel0: t14
// iChannel1: t5

precision highp float;

vec3 eyePos(float t)
{
    return vec3(0.0, 0.0, -20.0);
}

vec3 lightPos(float t)
{
    return eyePos(t) + vec3(4.0, 4.0, -5.0);   
}

float noise(float f)
{
	// Wonder how this works ? So do I.
	return fract(mod(f, 2.36843) * 337.5453);
}

float noise(vec2 f)
{
	// Wonder how this works ? So do I.
	return fract(dot(mod(f, vec2(2.36843, 2.48311)),
					        vec2(337.5453, 541.3476)));
}

float noise(vec3 f)
{
	// Wonder how this works ? So do I.
	return fract(dot(mod(f, vec3(2.36843, 2.48311, 2.51364)),
					        vec3(337.5453, 541.3476, 754.1254)));
}

float smooth_noise(float f)
{
	float t = fract(f);
	float c = floor(f);
	
	float k0 = noise(c);
	float k1 = noise(c + 1.0);
	
	return (1.0 - t) * k0 + t * k1;
}

float smooth_noise(vec2 f)
{
	vec2 t = fract(f);
	vec2 c = floor(f);
	
	float k0 = noise(c);
	float k1 = noise(c + vec2(1.0, 0.0));
	float k2 = noise(c + vec2(0.0, 1.0));
	float k3 = noise(c + vec2(1.0, 1.0));
	
	return (1.0 - t.y) * ((1.0 - t.x) * k0 + t.x * k1)  +
	       t.y         * ((1.0 - t.x) * k2 + t.x * k3);
}

float smooth_noise(vec3 f)
{
	vec3 t = fract(f);
	vec3 c = floor(f);
	
	float k0 = noise(c);
	float k1 = noise(c + vec3(1.0, 0.0, 0.0));
	float k2 = noise(c + vec3(0.0, 1.0, 0.0));
	float k3 = noise(c + vec3(1.0, 1.0, 0.0));
	
	float k4 = noise(c + vec3(0.0, 0.0, 1.0));
	float k5 = noise(c + vec3(1.0, 0.0, 1.0));
	float k6 = noise(c + vec3(0.0, 1.0, 1.0));
	float k7 = noise(c + vec3(1.0, 1.0, 1.0));
	
	return (1.0 - t.z) * ((1.0 - t.y) * ((1.0 - t.x) * k0 + t.x * k1)  +
	                      t.y         * ((1.0 - t.x) * k2 + t.x * k3)) +
	       t.z         * ((1.0 - t.y) * ((1.0 - t.x) * k4 + t.x * k5)  +
	                      t.y         * ((1.0 - t.x) * k6 + t.x * k7));
}

// Too slow, use smooth_noise instead
float level_noise(float f)
{
	return smooth_noise(f)             +
		   smooth_noise(f * 2.0) / 2.0 +
		   smooth_noise(f * 4.0) / 4.0 +
		   smooth_noise(f * 8.0) / 8.0;
}

float level_noise(vec2 f)
{
	return smooth_noise(f)             +
		   smooth_noise(f * 2.0) / 2.0 +
		   smooth_noise(f * 4.0) / 4.0 +
		   smooth_noise(f * 8.0) / 8.0;
}

float level_noise(vec3 f)
{
	return smooth_noise(f)             +
		   smooth_noise(f * 2.0) / 2.0 +
		   smooth_noise(f * 4.0) / 4.0 +
		   smooth_noise(f * 8.0) / 8.0;
}

vec3 rotate_x(vec3 p, float angle)
{
	return vec3(p.x,
				p.y * cos(angle) - p.z * sin(angle),
				p.y * sin(angle) + p.z * cos(angle));
}

vec3 rotate_y(vec3 p, float angle)
{
	return vec3(p.x * cos(angle) - p.z * sin(angle),
				p.y,
				p.x * sin(angle) + p.z * cos(angle));
}

vec3 rotate_z(vec3 p, float angle)
{
	return vec3(p.x * cos(angle) - p.y * sin(angle),
				p.x * sin(angle) + p.y * cos(angle),
			    p.z);
}

float box(vec3 p, float w, float h, float d)
{
	float x = max(p.x - w, -p.x);
	float y = max(p.y - h, -p.y);
	float z = max(p.z - d, -p.z);
	return max(x, max(y, z));
}

float sphere(vec3 p, float r)
{
	return length(p) - r;
}

float cylinder(vec3 p, float r, float h)
{
	float z = max(p.z - h, -p.z);
	return max(z, length(p.xy - vec2(r)) - r);
}

float fat_triangle(vec3 p, vec3 x, vec3 y, vec3 z, float eps)
{
	vec3 u = y - x;
	vec3 v = z - x;
	vec3 n = normalize(cross(u, v));
	
	vec3 t = p - x;
	float a = dot(t, u) / dot(u, u);
	float b = dot(t, v) / dot(v, v);
	float c = dot(t, n);
	
	float k0 = c - eps;
	float k1 = - c - eps;
	
	float k2 = a + b - 1.0;
	float k3 = max(-a, -b);
	
	return max(max(k0, k1), max(k2, k3));
}

float triangle(float t)
{
    return 2.0 * max(0.5 - abs(t - 0.5), 0.0);
}

float pillar(vec3 p)
{
    float angle = 0.7 * triangle(mod(iGlobalTime * 6.0, 5.0)) +
                  0.7 * triangle(mod(iGlobalTime * 6.0 + 1.0, 5.0)) +
                  0.6 * triangle(mod(iGlobalTime * 6.0, 7.0)) +
                  0.5 * triangle(mod(iGlobalTime * 6.0 + 1.0, 7.0));
    
	vec3 p1 = rotate_z(p, p.z * 0.5 + 0.8 * angle);
	float bottom = min(box(p1 + vec3(0.5, 0.5, 0.0), 1.5, 1.5, 3.0),
		               box(p1 + vec3(0.6, 0.6, 0.0), 1.7, 1.7, 2.0));
	vec3 p2 = rotate_z(p, p.z * 2.0 - 2.0 * angle);
	float top = cylinder(p2 + vec3(0.5,  0.5, -3.0), 0.4, 10.0);
	return min(top, bottom);
}

float pillar_grid(vec3 p)
{
	return pillar(vec3(mod(p.x, 10.0) - 5.0, mod(p.y, 10.0) - 5.0, p.z));
}

float ceiling(vec3 p)
{
	return 0.0;
}

float ground(vec3 p)
{
	return p.z + 5.0 - dot(texture2D(iChannel0, p.xy * 0.05), vec4(0.05));
}

float distance(vec3 p)
{	
	vec3 p2 = rotate_z(rotate_x(p , 3.14159 * 3.0 / 4.0), iGlobalTime);
	return min(ground(p2), pillar_grid(p2 + vec3(0.0, 0.0, 5.0)) + 0.1 * smooth_noise(10.0 * p2.yz));
}

vec3 normal(vec3 p)
{
	vec3 h = vec3(0.1, 0.0, 0.0);
	return normalize(vec3(distance(p + h.xyy) - distance(p - h.xyy),
				          distance(p + h.yxy) - distance(p - h.yxy),
				          distance(p + h.yyx) - distance(p - h.yyx)));
}

float occlusion(vec3 p, vec3 n)
{
	float a0 = distance(p + n * 0.1);
	float a1 = distance(p + n * 0.2);
	float a2 = distance(p + n * 0.3);
	float a3 = distance(p + n * 0.4);
	
	float k = 5.89;
	return 1.0 - k * (0.5    * abs(0.1 - a0) +
					  0.25   * abs(0.2 - a1) +
					  0.125  * abs(0.3 - a2) +
					  0.0625 * abs(0.4 - a3));
}

float shadow(vec3 p, vec3 l)
{
	// Smooth shadows (taken from http://www.iquilezles.org/www/articles/rmshadows/rmshadows.htm)
	float maxt = length(l - p);
	float mint = 0.05;
	
	vec3 dir = (l - p) / maxt;
	float t = mint;
	float res = 1.0;
	
	for (int i = 0; i < 48; i++)
	{
		float d = distance(p + dir * t);
		if (d < 0.005)
			return 0.0;
		res = min(res, 20.0 * d / t);
		t += d;
	}
	
	return res;
}

void main(void)
{
	vec3 eye = eyePos(iGlobalTime) + vec3(iMouse.xy / iResolution.xy, 0.0) * iMouse.z * 0.02;
	vec3 light = lightPos(iGlobalTime);
    vec2 scale = 1.0 * vec2(1.0, iResolution.y / iResolution.x);
    
	vec3 dir = vec3(scale.x * (2.0 * gl_FragCoord.x / iResolution.x - 1.0),
					scale.y * (2.0 * gl_FragCoord.y / iResolution.y - 1.0),
					1.0);
	
	// Find intersection
	float t = 0.0;
	for (int i = 0; i < 48; i++)
	{
		t += 0.8 * distance(eye + t * dir);
	}
	
	if (t >= 100.0)
	{
		// No intersection : background color
		gl_FragColor = vec4(0.0);
	}
	else
	{
		vec3 pos = eye + dir * t; 
		vec3 n = normal(pos);
	
		gl_FragColor = vec4(1) * (0.4 * occlusion(pos, n) +
								  0.4 * shadow(pos, light) *  dot(n, normalize(light - pos)));
	}
}
