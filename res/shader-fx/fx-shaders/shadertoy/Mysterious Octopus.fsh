// https://www.shadertoy.com/view/Ms2GWd
// Just wanted to play with deformation of the primitives (here: the tentacles).
#ifdef GL_ES
precision mediump float;
#endif

#define DELTA				0.02
#define RAY_LENGTH_MAX		150.0
#define RAY_STEP_MAX		200
#define AMBIENT				0.3
#define SPECULAR_POWER		5.0
#define SPECULAR_INTENSITY	0.3
#define FADE_POWER			3.0
#define GAMMA				0.8
#define M_PI				3.1415926535897932384626433832795

//#define ATAN2 // Comment this to use the original atan function
#define SHADOW

vec3 vRotateX (in vec3 p, in float angle) {
	float c = cos (angle);
	float s = sin (angle);
	return vec3 (p.x, c * p.y + s * p.z, c * p.z - s * p.y);
}

vec3 vRotateY (in vec3 p, in float angle) {
	float c = cos (angle);
	float s = sin (angle);
	return vec3 (c * p.x - s * p.z, p.y, c * p.z + s * p.x);
}

float fixDistance (in float d, in float correction, in float k) {
	correction = max (correction, 0.0);
	k = clamp (k, 0.0, 1.0);
	return min (d, max ((d - DELTA) * k + DELTA, d - correction));
}

float smin (float a, float b, float k) {

	// From http://www.iquilezles.org/www/articles/smin/smin.htm
	float h = clamp (0.5 + 0.5 * (b - a) / k, 0.0, 1.0);
	return mix (b, a, h) - k * h * (1.0 - h);
}

#ifndef ATAN2
#define atan2 atan
#else
float atan2 (float y, float x) {

	// From http://www.deepdyve.com/lp/institute-of-electrical-and-electronics-engineers/full-quadrant-approximations-for-the-arctangent-function-tips-and-V6yJDoI0iF
	float t1 = abs (y);
	float t2 = abs (x);
	float t3 = min (t1, t2) / max (t1, t2);
	t3 = t3 / (1.0 + 0.28086 * t3 * t3);
	t3 = t1 > t2 ? M_PI / 2.0 - t3 : t3;
	t3 = x < 0.0 ? M_PI - t3 : t3;
	t3 = y < 0.0 ? -t3 : t3;
	return t3;
}

float atan2_bis (float y, float x) {

	// From http://http.developer.nvidia.com/Cg/atan2.html
	float t1 = abs (y);
	float t2 = abs (x);
	float t3 = min (t1, t2) / max (t1, t2);
	float t4 = t3 * t3;
	float t5 = -0.013480470;
	t5 = t5 * t4 + 0.057477314;
	t5 = t5 * t4 - 0.121239071;
	t5 = t5 * t4 + 0.195635925;
	t5 = t5 * t4 - 0.332994597;
	t5 = t5 * t4 + 0.999995630;
	t3 = t5 * t3;
	t3 = t1 > t2 ? M_PI / 2.0 - t3 : t3;
	t3 = x < 0.0 ? M_PI - t3 : t3;
	t3 = y < 0.0 ? -t3 : t3;
	return t3;
}

float atan2_ter (float y, float x) {

	// From http://rc0rc0.wordpress.com/2013/06/05/minimax-approximation-to-arctan-atan-atan2/
	float t1 = abs (y);
	float t2 = abs (x);
	float t3 = min (t1, t2) / max (t1, t2);
	t3 = 0.97239 * t3 - 0.19195 * t3 * t3 * t3;
	t3 = t1 > t2 ? M_PI / 2.0 - t3 : t3;
	t3 = x < 0.0 ? M_PI - t3 : t3;
	t3 = y < 0.0 ? -t3 : t3;
	return t3;
}
#endif

float getDistance (in vec3 p) {
	float k = 0.2 + 0.2 * sin (p.y * 2.0 + iGlobalTime * 3.0);
	vec3 pp = p;
	pp.z *= 1.2;
	float body = length (pp) + k - 4.5;
	body = fixDistance (body, 2.0, 0.8);

	p.y += 1.0;
	float angle = 2.0 * M_PI / 8.0;
	pp = vRotateY (p, -angle * 0.5);
	angle *= floor (atan2 (pp.x, pp.z) / angle);
	p = vRotateY (p, angle);
	p.xy /= min (1.1, 40.0 / p.z);
	k = clamp (p.z - 5.0, 0.0, 0.5);
	p = vRotateX (p, p.z * (0.01 + 0.01 * sin (iGlobalTime * 0.5 + angle * 5.0)) + 0.1 * sin (p.z * 0.3 + iGlobalTime + angle * 3.0));
	p = vRotateY (p, 0.1 * sin (p.z * 0.2 + iGlobalTime * 2.0 + angle * 7.0));
	p.z = sin (p.z * 2.0) / 2.0;
	float tentacle = length (p) - 1.0 - k * sin (abs (p.x * p.y * p.y* p.y + p.z));
	tentacle = fixDistance (tentacle, 60.0, 0.15);

	return smin (body, tentacle, 0.2);
}

void main () {

	// Old monitor effect
	vec2 frag = 2.0 * gl_FragCoord.xy / iResolution.xy - 1.0;
	float round = pow (length (frag * frag), 6.0);
	frag.y *= 1.0 + round * 0.2;
	vec3 colorMonitor = vec3 (1.0 - round);
	colorMonitor *= vec3 (0.8, 1.0 + abs (mod (frag.y, 0.04) / 0.04 - 0.5), 0.8);

	// Define the ray corresponding to this fragment
	frag.x *= iResolution.x / iResolution.y;
	frag.x += 0.01 * sin (frag.y * 40.0 + iGlobalTime);
	vec3 direction = normalize (vec3 (frag, 2.0));

	// Set the camera
	vec3 origin = vec3 ((50.0 * cos (iGlobalTime * 0.1)), -3.0 + 10.0 * sin (iGlobalTime * 0.2), -35.0 + 20.0 * sin (iGlobalTime * 0.3));
	vec3 forward = -origin;
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
	for (int rayStep = 0; rayStep < RAY_STEP_MAX; ++rayStep) {
		dist = getDistance (p);
		rayLength += dist;
		if (dist < DELTA || rayLength > RAY_LENGTH_MAX) {
			break;
		}
		p = origin + direction * rayLength;
	}

	// Compute the background color
	vec3 lightDirection = normalize (vec3 (2.0, sin (iGlobalTime), -2.0));
	vec3 fragColor = vec3 (0.1, 0.2, 0.2 + 0.3 * max (0.0, dot (-direction, lightDirection)));
	if (dist < DELTA) {

		// Define the color of the octopus
		vec3 color = vec3 (0.3, 0.1, 0.2);

		// Initialize the lighting
		float fade = pow (1.0 - rayLength / RAY_LENGTH_MAX, FADE_POWER);
		float diffuse = 0.0;
		float specular = 0.0;

		#ifdef SHADOW
		// Ray march again to check whether the light is visible
		dist = RAY_LENGTH_MAX;
		rayLength = DELTA * 10.0;;
		for (int rayStep = 0; rayStep < RAY_STEP_MAX; ++rayStep) {
			vec3 pp = p + lightDirection * rayLength;
			dist = getDistance (pp);
			rayLength += dist;
			if (dist < DELTA || rayLength > RAY_LENGTH_MAX) {
				break;
			}
		}
		if (dist >= DELTA)
		#endif
		{
			// Get the normal
			vec2 h = vec2 (DELTA, 0.0);
			vec3 normal = normalize (vec3 (
				getDistance (p + h.xyy) - getDistance (p - h.xyy),
				getDistance (p + h.yxy) - getDistance (p - h.yxy),
				getDistance (p + h.yyx) - getDistance (p - h.yyx)));

			// Lighting
			diffuse = max (0.0, dot (normal, lightDirection));
			specular = pow (max (0.0, dot (reflect (direction, normal), lightDirection)), SPECULAR_POWER) * SPECULAR_INTENSITY;
		}

		// Compute the final color
		fragColor = mix (fragColor, (AMBIENT + diffuse) * color + specular, fade);
	}

	// Set the fragment color
	fragColor = pow (fragColor, vec3 (GAMMA));
	gl_FragColor = vec4 (fragColor * colorMonitor, 1.0);
}
