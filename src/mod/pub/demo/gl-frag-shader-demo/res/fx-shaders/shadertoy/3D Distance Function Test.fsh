// https://www.shadertoy.com/view/ldXSWj
const int MAX_STEPS = 16;
const vec3 BOX = vec3(1.0);
const float THRESHOLD = 0.01;

float udBox(vec3 p, vec3 b) {
	return length(max(abs(p) - b, 0.0));
}

struct Ray {
    vec3 position;
    vec3 direction;
    bool hit;
};

Ray intersect(Ray ray) {
	for (int i=0; i<MAX_STEPS; i++) {
		float dist = udBox(ray.position, BOX);
		if (dist < THRESHOLD) {
            ray.hit = true;
            return ray;
		}
		ray.position += ray.direction * dist;
	}
    return ray;
}

vec3 Rx(vec3 vector, float angle) {
    float rx = vector.x * cos(angle) + vector.z * sin(angle);
    float rz = vector.z * cos(angle) - vector.x * sin(angle);
    return vec3(rx, vector.y, rz);
}

vec3 render(Ray ray) {   
    if (ray.hit) {
        return (ray.position + BOX) / (2.0 * BOX);
    } else {
		return vec3(1.0);
    }
}

void main(void)
{
	float aspect_ratio = iResolution.x / iResolution.y;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 p = (uv - vec2(0.5)) * vec2(aspect_ratio, 1.0);
    
    float angle = iGlobalTime;
    
    if (iMouse.z > 0.0) {
    	angle = iMouse.x / iResolution.x * 6.0 + 0.5;
    }
    	
    vec3 camera_loc = 6.0 * vec3(-sin(angle), 0.0, -cos(angle));
    vec3 camera_dir = vec3(sin(angle), 0.0, cos(angle));
    
    Ray ray;
    ray.position = camera_loc;
    ray.direction = Rx(normalize(vec3(p, 1.0)), angle);
    ray.hit = false;
	
	ray = intersect(ray);
    
    vec3 col = render(ray);
    
	gl_FragColor = vec4(col, 1.0);
}
