// https://www.shadertoy.com/view/MslXz7
// Simple WebGL reference implementation of 2nd order directional SH lighting of a Lambertian surface. Per-light computation on the left, SH approximation on the right. Wipe with the mouse.
// 2nd order SH example by Morgan McGuire, http://graphicscodex.com
//
// See also:
//    http://dickyjim.wordpress.com/2013/09/04/spherical-harmonics-for-beginners/ 
//     (and dickyjim's example that this is based on at https://www.shadertoy.com/view/Xds3Rl)
//    http://home.comcast.net/~tom_forsyth/papers/SH_GDCE_TomF.zip
//    http://ppsloan.org/publications/StupidSH36.pdf
// 

#define Radiance3 vec3
#define Irradiance3 vec3
#define Vector3 vec3
#define Color3 vec3

// 2nd order SH
#define SH vec4

const float PI = 3.141592654;

// Increase this from 1.0 to increase terminator onset at the expense of ringing.
// Decrease towards 0.0 to blur lighting and reduce ringing and sharpness.
const float SHSharpness = 1.0;

struct Light {
	vec3 directionToLight;
	vec3 radiance;
};

// The actual lights
const int NUM_LIGHTS = 2;
Light lightArray[NUM_LIGHTS];

// The environment map contribution, which in this simple example
// is modeled as constant.
Radiance3 environment;

// A uniform flat environment color. This helps hide some of the SH
// ringing, which can force the environment term to zero.
Radiance3 ambient;

// SH approximation of all of the lights.
// Array index is color channel.
SH lightProbeSH[3];

const float SHCosWtIntOrder2 = 0.75;
const float SHDirectionalLightNormalizationFactor = PI / SHCosWtIntOrder2;

SH projectToSH(Vector3 direction) {	
	const float p_1_1 = -0.488602511902919920;	
	return vec4(
		// l=0, m=0	
		0.282094791773878140,

		// l=1, m=0
		0.488602511902919920 * direction.z,
		
		// l=1, m=-1
		p_1_1 * direction.y,

		// l=1, m=+1
		p_1_1 * direction.x);
}



void convertToIrradianceProbe(inout SH sh_l) {
	const float sh_c0 = (2.0 - SHSharpness) * 1.0;
	const float sh_c1 = SHSharpness * 2.0 / 3.0;
	sh_l *= vec4(sh_c0, vec3(sh_c1));
}


///////////////////////////////////////////////////////////////////////
// Lighting

/* In a real application, this step would be precomputed (per frame or per scene)
   for each SH probe */	
void computeSHLightProbe(Light lightArray[NUM_LIGHTS]) {
	for (int c = 0; c < 3; ++c) {
		lightProbeSH[c] = vec4(environment[c], vec3(0));
	}
	
	for (int i = 0; i < NUM_LIGHTS; ++i) {
		vec4 lightSH = projectToSH(lightArray[i].directionToLight) * SHDirectionalLightNormalizationFactor;
	
		// Compute the probe
		for (int c = 0; c < 3; ++c) {
			lightProbeSH[c] += lightSH * lightArray[i].radiance[c];
		}
	}

	for (int c = 0; c < 3; ++c) {
		convertToIrradianceProbe(lightProbeSH[c]);
	}
}


/* Per-pixel shading */
Radiance3 shade(SH lightProbeSH[3], Vector3 normal, Color3 surfaceReflectivity) {
	vec4 normalSH = projectToSH(normal);

	// This is a 4x3 x 3x1 matrix product, but WebGL can't
	// express that.
	Irradiance3 result;
	for (int c = 0; c < 3; ++c) {
		result[c] = dot(lightProbeSH[c], normalSH);
	}
	
	// Better results could be obtained by handling the ambient term separately
	// so that ringing can't force the color to black.
	return (max(result, vec3(0)) + ambient) * surfaceReflectivity / PI;
}


///////////////////////////////////////////////////////////////////////
// Shadertoy setup


/* Returns w=0 if the ray misses */
vec4 intersectSphere(vec2 pix, float size) {
	vec2 uv = pix / size;
	float offset = uv.x * uv.x + uv.y * uv.y;
	return (offset > 1.0) ? vec4(0.0) : vec4(uv, sqrt(1.0 - offset), 1.0);
}


Radiance3 shade2(Light lightArray[NUM_LIGHTS], Vector3 normal, Color3 surfaceReflectivity) {
	Irradiance3 result = ambient + environment;

	for (int i = 0; i < NUM_LIGHTS; ++i) {
		result += max(dot(lightArray[i].directionToLight, normal), 0.0) * lightArray[i].radiance;
	}
		
	return result * surfaceReflectivity / PI;
}

	
void main(void) {
	// These are directional lights, but we could instead evaluate point lights
	// into directional lights at the location of the SH light probe.
	lightArray[0] = Light(normalize(vec3(cos(iGlobalTime * 0.2 + 2.0),1,0.1)), 1.0 * Radiance3(0.75,0.75,0.75));
	lightArray[1] = Light(normalize(vec3(1,cos(iGlobalTime),sin(iGlobalTime))), 1.2 * Radiance3(0.75,0.6,0));
	environment = Radiance3(0.05, 0.1, 0.1);
	ambient = Radiance3(0.1);
	
	// Click near the top to simplify the lighting rig
	if (iMouse.y > iResolution.y * 0.9) {
		lightArray[1].radiance *= 0.0;
		environment *= 0.0;
		ambient *= 0.0;
	}

	computeSHLightProbe(lightArray);	


	// Fraction of light reflected, on [0, 1]. The integral of f(wi,wo)*|n . wi| over the hemisphere.
	const Color3 surfaceReflectivity = vec3(1);

	vec2 pix = vec2(mod(gl_FragCoord.x, iResolution.x * 0.5) - iResolution.x * 0.25,
           		gl_FragCoord.y - iResolution.y * 0.5);
	pix *= 0.6;
	
	vec4 position = intersectSphere(pix, 0.25 * min(iResolution.x, iResolution.y));
	
	// For a sphere about the origin, the position is the normal
	vec3 normal = normalize(position.xyz);
	
	float mouseX = (iMouse.x > 0.0) ? iMouse.x : iResolution.x * 0.5;
	
	Radiance3 L = (gl_FragCoord.x > mouseX) ? 
		shade(lightProbeSH, normal, surfaceReflectivity) :
		shade2(lightArray, normal, surfaceReflectivity);
	
	const float gamma = 2.1;	
	
	gl_FragColor = vec4(
		(abs(gl_FragCoord.x - mouseX) < 1.0) ? 
		  vec3(1.0) :
		  pow(mix(Radiance3(0.3 + 0.3 * gl_FragCoord.y / iResolution.y, 0.5, 0.35 - 0.05 * gl_FragCoord.y / iResolution.y), L, position.w), vec3(1.0 / gamma)), 
		1.0);
}
