// https://www.shadertoy.com/view/Msj3Wt
#ifdef GL_ES
precision mediump float;
#endif

#define DELTA			0.01
#define RAY_LENGTH_MAX	300.0
#define RAY_STEP_MAX	200

float fixDistance (in float d, in float correction, in float k) {
	correction = max (correction, 0.0);
	k = clamp (k, 0.0, 1.0);
	return min (d, max ((d - DELTA) * k + DELTA, d - correction));
}

float getDistance (in vec3 p) {
	p += vec3 (3.0 * sin (p.z * 0.2 + iGlobalTime * 2.0), sin (p.z * 0.3 + iGlobalTime), 0.0);
	return fixDistance (length (p.xy) - 4.0 + 0.8 * sin (abs (p.x * p.y) + p.z * 4.0) * sin (p.z), 2.5, 0.2);
}

void main () {

	// Define the ray corresponding to this fragment
	vec2 frag = (2.0 * gl_FragCoord.xy - iResolution.xy) / iResolution.y;
	vec3 direction = normalize (vec3 (frag, 2.0));

	// Set the camera
	vec3 origin = vec3 ((17.0 + 5.0 * sin (iGlobalTime)) * cos (iGlobalTime * 0.2), 12.0 * sin (iGlobalTime * 0.2), 0.0);
	vec3 forward = vec3 (-origin.x, -origin.y, 22.0 + 6.0 * cos (iGlobalTime * 0.2));
	vec3 up = vec3 (0.0, 1.0, 0.0);
	mat3 rotation;
	rotation [2] = normalize (forward);
	rotation [0] = normalize (cross (up, forward));
	rotation [1] = cross (rotation [2], rotation [0]);
	direction = rotation * direction;

	// Ray marching
	vec3 p = origin;
	float dist = RAY_LENGTH_MAX;
	float rayLength = 0.0;
	int stepCount = 0;
	for (int rayStep = 0; rayStep < RAY_STEP_MAX; ++rayStep) {
		dist = getDistance (p);
		rayLength += dist;
		if (dist < DELTA || rayLength > RAY_LENGTH_MAX) {
			break;
		}
		p = origin + direction * rayLength;
		++stepCount;
	}

	// Compute the fragment color
	vec4 color = vec4 (float (stepCount * 3) / float (RAY_STEP_MAX), float (stepCount) * 1.5 / float (RAY_STEP_MAX), 0.0, 1.0);
	vec3 LIGHT = normalize (vec3 (1.0, -3.0, -1.0));
	if (dist < DELTA) {
		vec2 h = vec2 (DELTA, 0.0);
		vec3 normal = normalize (vec3 (
			getDistance (p + h.xyy) - getDistance (p - h.xyy),
			getDistance (p + h.yxy) - getDistance (p - h.yxy),
			getDistance (p + h.yyx) - getDistance (p - h.yyx)));
		color.rg += 0.5 * max (0.0, dot (normal, LIGHT));
	}
	else {
		color.b += 0.1 + 0.5 * max (0.0, dot (-direction, LIGHT));
	}
	gl_FragColor = color;
}
