// https://www.shadertoy.com/view/ldsSW7

// iChannel0: t14
// iChannel1: t11
// iChannel2: c0

// lens testing from previous raymarcher source. refract_koef above 1.0 makes the concave lens and below 1.0 - a convex one.
#define R 2.0

#define refract_koef 1.02

#define speed 2.0
#define max_distance 300.0
#define epsilon 0.001
#define max_steps 500
#define K 0.37
#define shininess 1.0
#define ambient 0.4
#define bump_factor 0.0
#define specular_koef 40.5
#define diffuse_koef 2.0
#define FOV 60.0

float ball_vertical = 0.0;

// returns height of water at time t
float water(vec3 point, float t) {
	float k = 0.6;
	float h1 = sin(point.x)*1.0;
	float h2 = sin(point.y)*2.0;
	float h3 = sin(point.z)*2.0;
	float h4 = cos(t+point.z+point.x+point.y);
	
	float combined = h1 + h2 + h3 + h4;
	combined = combined / 4.0;
	return combined *k;
}

vec4 texture3d (sampler2D t, vec3 p, vec3 n, float scale) {
	return 
		texture2D(t, p.yz * scale) * abs (n.x) +
		texture2D(t, p.xz * scale) * abs (n.y) +
		texture2D(t, p.xy * scale) * abs (n.z);
}

float smin( float a, float b, float k) {
    float res = exp( -k*a ) + exp( -k*b );
    return -log( res )/k;
}

float get_distance(vec3 point) {
	float bump = 0.0;
	float elevation = -1.0;
	
	if ( length(point) < R + bump_factor) {
		bump = bump_factor * texture3d(iChannel1, point, normalize(-point), 0.5).r;
	}	
	
	vec3 ball_point = point;
	ball_point.y += ball_vertical;
	ball_point.y += 0.4;
	
	return length(ball_point) - R + bump;
		//dot(point, vec3(0.0, 1.0, 0.0)) - elevation,
		
}

float raymarch(vec3 ray_origin, vec3 ray_direction) {
	float d = 0.0;
	
	for (int i = 0; i < max_steps; i++) {
		vec3 new_point = ray_origin + ray_direction*d;
		float s = get_distance(new_point);
		if (s < epsilon) return d;
		d += s;
		if (d > max_distance) return max_distance;
	}
	return max_distance;
}

vec3 get_normal(vec3 point) {
	float d0 = get_distance(point);
	float dX = get_distance(point-vec3(epsilon, 0.0, 0.0));
	float dY = get_distance(point-vec3(0.0, epsilon, 0.0));
	float dZ = get_distance(point-vec3(0.0, 0.0, epsilon));
		
	return normalize(vec3(dX-d0, dY-d0, dZ-d0));
}

mat3 rotateY(float fi) {
	return mat3(
	    cos(fi), 0.0, sin(fi),
	  	0.0, 1.0, 0.0,
		-sin(fi), 0.0, cos(fi)
	);
}

mat3 rotateX(float fi) {
	return mat3(
		1.0, 0.0, 0.0,
		0.0, cos(fi), -sin(fi),
		0.0, sin(fi), cos(fi)
	);	
}

float shadow_sample (vec3 org, vec3 dir) {
    float res = 1.0;
    float t = epsilon*200.0;
    for (int i =0; i < 100; ++i){
        float h = get_distance (org + dir*t);
		if (h <= epsilon) {
            return 0.0;
		}
        res = min (res, 32.0*h/t);
        t += h;
		if (t >= max_distance) {
      		return res;
		}
		
    }
    return res;
}

void main(void)
{
	ball_vertical = 0.3*sin(iGlobalTime*speed);
	vec2 uv = ((2.0 * gl_FragCoord.xy) - iResolution.xy) / min(iResolution.x, iResolution.y);
	uv *= tan (radians (FOV)/2.0);
	
	vec4 color = vec4(0.0);
	
	vec3 light = vec3(-1.0, 4.0, 8.0) * rotateY(iGlobalTime*2.0);
	
	mat3 rotated = rotateY(sin(iGlobalTime/4.0));
	
	vec3 eye_pos = rotated*vec3(0.0, 0.0, -5.0);
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 forward = rotated*vec3(0.0, 0.0, 1.0);
	vec3 right = cross(up, forward);	
	
	vec3 ray_dir = normalize(up * uv.y + right *uv.x + forward);

	float d = raymarch(eye_pos, ray_dir);
	vec3 point = (eye_pos+ray_dir*d);			
	
	if (d < max_distance) {		
		vec3 point_normal = get_normal(point);
		
		vec3 light_dir = -normalize(light-point);
		vec3 reflected_light_dir = reflect(-light_dir, point_normal);
		float attenuation = 1.0 / (1.0 + K*pow( length(light - point), 2.0));
			
		float dotp_diffuse = max(0.0, dot(light_dir, point_normal));
		float dotp_specular = pow(max(0.0, dot(ray_dir, reflected_light_dir)), shininess);
		
		// no diffuse -> no specular
		if (dotp_diffuse <= 0.0) dotp_specular = 0.0;
		
		//gl_FragColor = vec4(0.2, 0.2, 0.6, 1.0) * (ambient + (dotp_diffuse*diffuse_koef + dotp_specular*specular_koef) * shadow_sample(point, -light_dir) * attenuation);// * texture3d(iChannel0, point, point_normal, 0.5);
		vec3 reflected_ray = reflect(ray_dir, point_normal);		
		vec3 refracted_ray = refract(ray_dir, -point_normal, refract_koef);
		
		vec4 reflected_color = textureCube(iChannel2, reflected_ray);
		vec4 refracted_color = textureCube(iChannel2, refracted_ray);		
		
		vec4 mixed = mix(reflected_color, refracted_color, 0.5);
		mixed = refracted_color;
		gl_FragColor = mixed;
	} else {			
	    gl_FragColor = textureCube(iChannel2, ray_dir);
	}
}
