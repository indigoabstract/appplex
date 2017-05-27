// https://www.shadertoy.com/view/lssGzX

// iChannel0: t7

vec3 light = vec3(-1, -1, 1.5);
vec3 light_color = vec3(1, 1, 1);

vec3 sphere = vec3(0, 0, 2);
float sphere_size = 1.3;
vec3 sphere_color = vec3(1, 1, 1);

float raySphere(vec3 rpos, vec3 rdir, vec3 sp, float radius, inout vec3 point, inout vec3 normal) {
	radius = radius * radius;
	float dt = dot(rdir, sp - rpos);
	if (dt < 0.0) {
		return -1.0;
	}
	vec3 tmp = rpos - sp;
	tmp.x = dot(tmp, tmp);
	tmp.x = tmp.x - dt*dt;
	if (tmp.x >= radius) {
		return -1.0;
	}
	dt = dt - sqrt(radius - tmp.x);
	point = rpos + rdir * dt;
	normal = normalize(point - sp);
	return dt;
}

void main(void) {
	light.xy = iMouse.xy / iResolution.xy * 2.0 - 1.0;
	light.y = -light.y;
	vec3 point; 
	vec3 normal;
	vec2 uv = gl_FragCoord.xy / iResolution.xy * 2.0 - 1.0;
	uv.x *= iResolution.x / iResolution.y;
	uv.y = -uv.y;
	vec3 ray = vec3(uv.x, uv.y, 1.0);
	ray = normalize(ray);
	gl_FragColor = vec4(0.0);
	float dist = raySphere(vec3(0.0), ray, sphere, sphere_size, point, normal);
	
	if (dist > 0.0) {
		vec3 tmp = normalize(light - sphere);
		float u = atan(normal.z, normal.x) / 3.1415*2.0 + iGlobalTime / 5.0;
		float v = asin(normal.y) / 3.1415*2.0 + 0.5;
		
		normal = normalize(normal * texture2D(iChannel0, vec2(u, v)).xyz);
		
		u = atan(normal.z, normal.x) / 3.1415*2.0 + iGlobalTime / 5.0;
		v = asin(normal.y) / 3.1415*2.0 + 0.5;
		gl_FragColor.xyz = vec3(dot(tmp, normal)) * light_color * sphere_color * texture2D(iChannel0, vec2(u, v)).xyz;
	}
}
