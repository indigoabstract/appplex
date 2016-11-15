// https://www.shadertoy.com/view/XsXXDM
//Based on https://www.shadertoy.com/view/MdXSD8

#define SPHERE_SIZE 3.0

//Light setup
vec3 light = vec3(0, 0.0, 25.0);

//Functions 

mat4 rotateY(float theta) {
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);
	return mat4(cosTheta, 0.0, sinTheta, 0.0,
				0.0, 1.0, 0.0, 0.0,
				-sinTheta, 0.0, cosTheta, 0.0,
				0.0, 0.0, 0.0, 1.0);
}

vec2 iSphere(in vec3 rayOrigin, in vec3 rayDirection, in vec4 sph) {
	//sphere at origin has equation |xyz| = r
	//so |xyz|^2 = r^2.
	//Since |xyz| = rayOrigin + t*rayDirection (where t is the distance to move along the ray),
	//we have rayOrigin^2 + 2*rayOrigin*t*rayDirection + t^2 - r^2. This is a quadratic equation, so:
	vec3 oc = rayOrigin - sph.xyz; //distance ray origin - sphere center
	
	float b = dot(oc, rayDirection);
	float c = dot(oc, oc) - sph.w * sph.w; //sph.w is radius
	float h = b*b - c; //Commonly known as delta. The term a is 1 so is not included.
	
	vec2 t;
	if(h < 0.0) 
		t = vec2(-1.0);
	else  {
		float sqrtH = sqrt(h);
		t.x = (-b - sqrtH); //Again a = 1.
		t.y = (-b + sqrtH);
	}
	return t;
}

//Get sphere normal.
vec3 nSphere(in vec3 pos, in vec4 sph ) {
	return normalize((pos - sph.xyz)/sph.w);
}

float intersect(in vec3 rayOrigin, in vec3 rayDirection, out vec2 resT, out vec4 sph) {
	resT = vec2(1000.0);
	float hitId = -1.0;
	float sphId = hitId;
	mat4 rotationAngle = rotateY(-0.1 * iGlobalTime);
	
	//check against spheres in the scene
	for (float x = -20.0; x <= 20.0; x += 10.0) {
		for (float y = -20.0; y <= 20.0; y += 10.0) {
			for (float z = -20.0; z <= 20.0; z += 10.0) {
				vec3 pos = vec3(x, y, z);
				sphId += 1.0;
				vec4 posSph = vec4(pos, 1.0);
				posSph = rotationAngle * posSph;
				vec4 sphTry = vec4(posSph.x + 0.5*posSph.x*sin(0.3*iGlobalTime), 
								   posSph.y + 0.25*posSph.y*sin(0.4*iGlobalTime),
								   posSph.z + 0.25*posSph.z*sin(0.5*iGlobalTime),
								   SPHERE_SIZE);
				vec2 tsph = iSphere(rayOrigin, rayDirection, sphTry);
				if(tsph.x > 0.0 && resT.y > tsph.x) {
					sph = sphTry;
					resT = tsph;
					hitId = sphId;
				}
			}
		}
	}
	
	//check against the light
	vec4 lightSph = vec4(light, 0.5);
	vec2 tsph = iSphere(rayOrigin, rayDirection, lightSph);
	if (tsph.x > 0.0 && resT.y > tsph.x) {
		sph = lightSph;
		resT = tsph;
		hitId = -2.0;
	}
	return hitId;
}

void main(void) {
	//pixel coordinates from 0 to 1
	float aspectRatio = iResolution.x/iResolution.y;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	//generate a ray with origin ro and direction rd
	vec3 rayOrigin = vec3(0.0, 0.0, 40.0);
	vec3 rayDirection = normalize(vec3( (-1.0+2.0*uv) * vec2(aspectRatio, 1.0), -1.0));
	
	mat4 rotY = rotateY(iGlobalTime);
	light.y += 10.0*sin(iGlobalTime);
	light = (vec4(light, 1.0) * rotY).xyz;

	//intersect the ray with scene
	vec2 t;
	vec4 sphHit;
	float id = intersect(rayOrigin, rayDirection, t, sphHit);
	
	vec4 col = vec4(vec3(0.1), 1.0);
	//If we hit a sphere
	if(id >= 0.0)
	{
		//find the point where we hit the sphere and evaluate luminance
		vec3 pos = rayOrigin + t.x*rayDirection;
		vec3 nor = nSphere(pos, sphHit);
		float dif = clamp(dot(nor, normalize(light-pos)), 0.0, 1.0);
		col = vec4(vec3(dif), 1.0);
		
		//check to see if this point is in shadow
		vec2 shadowT;
		vec4 shadowHit;
		//check for intersect between the sphere and the light
		float shadowId = intersect(pos, normalize(light-pos), shadowT, shadowHit);
		
		//if we have a non-negative id, we've hit something other than the light
		if (shadowId >= 0.0) {
			col = vec4(0.0);
		}
	}
	//If we hit the light
	else if (id == -2.0) {
		col = vec4(1.0);
	}
	
	gl_FragColor = col;
}
