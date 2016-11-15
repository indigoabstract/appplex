// https://www.shadertoy.com/view/Mds3DM

// iChannel0: c0

const float dl = .2;
const float tanfov = 3.;
const float blackHoleDist = 2.;
const float blackHoleMass = .1;	//2M = R = Schwarzschild radius
const int maxiter = 30;

const mat3 viewMatrix = mat3(
	0., 0., -1.,
	1., 0., 0., 
	0., -1., 0.);
const mat3 viewMatrixInv = mat3(
	0., 1., 0.,
	0., 0., -1.,
	-1., 0., 0.);

vec3 rotate(const vec3 v, float theta, const vec3 axis) {
	float cosTheta = cos(theta);
	return v * cosTheta + cross(axis, v) * sin(theta) + axis * dot(v, axis) * (1. - cosTheta);
}

vec4 calcRayAccel(vec4 pos, vec4 vel) {	
	//calculate schwarzschild geodesic acceleration
	float r = length(pos.xyz);
	float oneMinus2MOverR = 1. - 2.*blackHoleMass/r;			
	float posDotVel = dot(pos.xyz, vel.xyz);
	float velDotVel = dot(vel.xyz, vel.xyz);
	float r2 = r * r;
	float invR2M = 1. / (r * oneMinus2MOverR);
	float rMinus2MOverR2 = oneMinus2MOverR / r;
	float MOverR2 = blackHoleMass / r2;
	vec4 accel = vec4(0.);
	accel.xyz = -MOverR2 * (rMinus2MOverR2 * pos.xyz * vel.w * vel.w + invR2M * (pos.xyz * velDotVel - 2. * vel.xyz * posDotVel));
	accel.w = 2. * MOverR2 * invR2M * posDotVel * vel.w;
	return accel;
}

void main() {

	//get screen coords	
	vec2 uv = gl_FragCoord.xy / vec2(iResolution.xy);
	uv -= .5;
	uv *= tanfov;
	
	//rotate z- to x+
	vec3 uv3 = normalize(viewMatrixInv * vec3(uv, -1.));
	
	//start us off
	vec4 pos = vec4(0.);
	vec4 vel = vec4(uv3, 0.);
	//offset position
	pos.x -= blackHoleDist;
	//normalize velocity
	float r = length(pos.xyz);
	vel.w = 1. / (1. - 2. * blackHoleMass / r);
		
	for (int i = 0; i < maxiter; ++i) {
		/* Euler integration * /
		vec4 accel = calcRayAccel(pos, vel);
		vec4 newpos = pos + vel * dl;
		vec4 newvel = vel + accel * dl;
		/**/
		/* Runge-Kutta 4 integration */
		vec4 accel1 = calcRayAccel(pos, vel);
		vec4 vel2 = vel + .5 * dl * accel1;
		vec4 accel2 = calcRayAccel(pos + .5 * dl * vel, vel2);
		vec4 vel3 = vel + .5 * dl * accel2;
		vec4 accel3 = calcRayAccel(pos + .5 * dl * vel2, vel3);
		vec4 vel4 = vel + dl * accel3;
		vec4 accel4 = calcRayAccel(pos + dl * vel3, vel4);
		vec4 newpos = pos + (vel + 2. * vel2 + 2. * vel3 + vel4) * dl / 6.;
		vec4 newvel = vel + (accel1 + 2. * accel2 + 2. * accel3 + accel4) * dl / 6.;
		/**/
		
		pos = newpos;
		vel = newvel;
	}

	//rotate x+ to z-
	vec3 cubeTexCoord = viewMatrix * vel.xyz;

	//give texcoords some rotation
	cubeTexCoord = rotate(cubeTexCoord, iGlobalTime * .2, vec3(0., 1., 0.));
	
	vec4 color = textureCube(iChannel0, cubeTexCoord);
	
	//if we failed to escape ...
	r = length(pos.xyz);
	if (r < blackHoleMass * 2.) {
		color = vec4(0., 0., 0., 1.);
	} else {
	//otherwise, shift frequency
		
		//lambda = lambda0 * (1 - blackHoleMass / r);
	}
	
	gl_FragColor = color;
}
