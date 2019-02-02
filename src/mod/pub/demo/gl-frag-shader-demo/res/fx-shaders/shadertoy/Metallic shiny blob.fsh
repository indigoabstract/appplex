// https://www.shadertoy.com/view/4sl3Df

// iChannel0: t7

#define PI 3.14159
#define EPS 0.001


vec3 backgroundColor = vec3(0.0, 0.0, 0.0);
vec3 lightDir        = normalize(vec3(0.0, 1.0, 1.0));
vec3 cam_pos         = vec3(2.5, 7.0, -8.0);
vec3 cam_target      = vec3(0.0, 0.0, 0.0);
vec3 world_up        = vec3(0.0, 1.0, 0.0);
		

float distanceSphere(vec3 aPosition, float radius) {
	return length(aPosition) - radius;	
}

float distanceFunc(vec3 aPosition) {
	float sphere2 = distanceSphere(aPosition, 7.0);		
	float defsphere2 = sphere2 +
		sin(aPosition.x * 5.0 + iGlobalTime) / 4.0 +
		sin(aPosition.z * 5.0 + iGlobalTime) / 4.0;		
	return defsphere2;
}

vec3 shootRay(vec3 aPosition, vec3 aForward) {
	bool ok = false;
	
	for (int i = 0; i < 64; ++i) {		
		float distance = distanceFunc(aPosition);
		if (distance < EPS) {
			ok = true;
			continue;
		}
		else aPosition += aForward * distance / 2.0;
	}
		
	if (!ok) return backgroundColor;
	else {
		vec3 normal;
		normal.x = distanceFunc(aPosition + vec3(EPS, 0, 0)) - distanceFunc(aPosition - vec3(EPS, 0, 0));
		normal.y = distanceFunc(aPosition + vec3(0, EPS, 0)) - distanceFunc(aPosition - vec3(0, EPS, 0));
		normal.z = distanceFunc(aPosition + vec3(0, 0, EPS)) - distanceFunc(aPosition - vec3(0, 0, EPS));		
		normal = normalize(normal);	
		
		float diffuse = max(0.0, dot(-aForward, normal));
		float specular = max(pow(diffuse, 64.0), 0.0);
		
		vec2 texcoord = vec2(
			atan(normal.z / normal.x),
			acos(normal.y)
		);
		
		vec3 tex = texture2D(iChannel0, texcoord).rgb * (diffuse + specular);
		return vec3(tex);		
	}		
}


void main(void) {

	vec2 uv = gl_FragCoord.xy / iResolution.xy * 2.0 - 1.0;	// -1 .. 1
	float aspect = iResolution.x / iResolution.y;
	vec2 mp = iMouse.xy / iResolution.xy * 2.0 - 1.0;	// -1 .. 1	
	
	cam_target = vec3(0.0);
	float rad = iGlobalTime / 10.0; 
	//cam_pos.y = 5.0; //5.0+mp.y*5.0;
	//cam_pos.x = sin(rad) * 10.0;
	//cam_pos.z = cos(rad) * 10.0;
	
		
	vec3 cam_forward  = normalize(cam_target - cam_pos);
	vec3 cam_sideways = normalize(cross(world_up, cam_forward));
	vec3 cam_up       = normalize(cross(cam_forward, cam_sideways));
	
	
	vec3 disp = cam_up * uv.y + cam_sideways * uv.x * aspect;
	
	gl_FragColor = vec4(		
		shootRay(
				cam_pos, 
				normalize(cam_forward + disp)
		), 1.0
	);	 
		
	
}
