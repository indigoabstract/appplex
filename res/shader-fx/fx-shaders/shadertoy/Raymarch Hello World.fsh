// https://www.shadertoy.com/view/4dXXW7
// My first attempt at understanding raymarching. 
// Completely inefficient but by jove it works!


const int RAY_STEPS = 125;
const float cubeSize = 4.0;
const float speed = 30.0;


void main(void) {
	// uv coord
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	// camera stuff
	vec3 camPos = vec3(15.0, 8.0,15.0);
	vec3 camNorm = vec3(0.653, 0.383, 0.653);
	//animated spin camera
	float rads = iGlobalTime * speed * 0.01745;
	mat3 spin = mat3(vec3(cos(rads), 0.0, -sin(rads)), vec3(0.0, 1.0, 0.0), vec3(sin(rads), 0.0, cos(rads)));
	camPos = spin * camPos;
	camNorm = spin * camNorm;
	// camera attributes
	float camNear = 5.0;
	float camFar = 35.0;
	float camHFOV = 65.0;
	float camVFOV = 35.0;
	// camera matrix
	vec3 camXAxis = cross(vec3(0.0, 1.0, 0.0), camNorm);
	camXAxis =  normalize(camXAxis);
	vec3 camYAxis = cross(camNorm, camXAxis);
	mat3 camRotMat = mat3(camXAxis, camYAxis, camNorm);
	
	//ray vector
	vec3 raySt = vec3(0.0, 0.0, -camNear);
	float nearW = tan(camHFOV * .5 * .01745) * abs(camNear);
	raySt.x = (nearW * uv.s * 2.0) - nearW;
	float nearH = tan(camVFOV * .5 * .01745) * abs(camNear);
	raySt.y = (nearH * uv.t * 2.0) - nearH;
	// run the ray through the cameras rotation matrix, and add it's position
	raySt = (camRotMat * raySt) + camPos;
	vec3 rayDir = normalize(raySt - camPos);
	vec3 rayInc = rayDir * ((abs(camFar) - abs(camNear)) / float(RAY_STEPS));
	
	// march
	vec3 result = vec3(1.0, 0.0, 1.0);
	vec3 curPos = raySt;
	for (int i = 0; i < RAY_STEPS; ++i) {
		if (i == RAY_STEPS - 1) {
			result = mix(vec3(0.15, 0.15, 0.15), vec3(0.6, 0.05, 0.8), uv.t);			
		}
		if (curPos.x > -cubeSize && curPos.x < cubeSize && curPos.y > -cubeSize && curPos.y < cubeSize && curPos.z > -cubeSize && curPos.z < cubeSize ) {
			//return curPos;
			result = vec3(0.0,  float(i) / float(RAY_STEPS) * 1.5, 0.0);
			break;
		}
		curPos += rayInc;
	}
	

	gl_FragColor = vec4(result, 1.0);
}
