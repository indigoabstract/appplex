// https://www.shadertoy.com/view/lsfXWM

// iChannel0: t14
// iChannel1: t1

// A little ray tracing experiment with colored light transmittance and Fresnel reflectance with Voronoi "stained glass". Mouse left/right to rotate the scene.
#define M_PI 3.1415926535

struct ray {
	vec3 origin;
	vec3 direction;
	vec4 lightColor;
	float transmittance;
	float rayLength;
	int lastHitObject;
};


//Camera position setup
vec3 startingLocation = vec3(0.0, 5.0, 15.0);
	
//Floor setup
const vec4 sceneFloor = vec4(0.0, 1.0, 0.0, 0.0);
const float floorTransmission = 0.7;

//Window setup
const vec4 sceneWindow = vec4(-1.0, 0.0, 0.0, -7.5);
const vec2 windowStart = vec2(-10.0, 1.0);
const vec2 windowWidthHeight = vec2(20.0, 15.0);
const float glassTransmission = 0.5;

//Random meaningless sphere setup
const vec4 sceneSphere = vec4(0.0, 3.0, 3.0, 3.0);
const float sphereTransmission = 0.5;

//Light setup
vec4 light = vec4(-480.0, 0.0, 0.0, 110.0);

const float tau = 6.28318530717958647692;

// Using modified Fast Voronoi Pattern (https://www.shadertoy.com/view/Msl3Rl) for the stained glass effect
// Created by Ben Weston - 2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
vec4 Rand( ivec2 pos ) {
	return texture2D( iChannel0, (.5+vec2(pos.xy))/256.0, -100.0 );
}

vec3 Voronoi( vec2 pos ) {
	vec2 d[8];
	d[0] = vec2(0);
	d[1] = vec2(1,0);
	d[2] = vec2(0,1);
	d[3] = vec2(1);
	
	const float maxDisplacement = .7;//.518; //tweak this to hide grid artefacts
	
	float closest = 12.0;
	vec4 result;
	for ( int i=0; i < 8; i++ )
	{
		vec4 r = Rand(ivec2(floor(pos+d[i])));
		vec2 p = d[i] + maxDisplacement*(r.xy-.5);
		p -= fract(pos);
		float lsq = dot(p,p);
		if ( lsq < closest )
		{
			closest = lsq;
			result = r;
		}
	}
	return fract(result.xyz+result.www); // random colour
}

mat4 rotateX(float theta) {
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);
	return mat4(1.0, 0.0, 0.0, 0.0,
				0.0, cosTheta, -sinTheta, 0.0,
				0.0, sinTheta, cosTheta, 0.0,
				0.0, 0.0, 0.0, 1.0);
}

mat4 rotateY(float theta) {
	float cosTheta = cos(theta);
	float sinTheta = sin(theta);
	return mat4(cosTheta, 0.0, sinTheta, 0.0,
				0.0, 1.0, 0.0, 0.0,
				-sinTheta, 0.0, cosTheta, 0.0,
				0.0, 0.0, 0.0, 1.0);
}

mat4 translate(vec3 v) {
	return mat4(1.0, 0.0, 0.0, v.x,
				0.0, 1.0, 0.0, v.y,
				0.0, 0.0, 1.0, v.z,
				0.0, 0.0, 0.0, 1.0);
}

float iPlane(in ray sceneRay, in vec4 plane) {
	return -(dot(sceneRay.origin, normalize(plane.xyz)) + plane.w) / dot(sceneRay.direction, plane.xyz);
}

float iRectangle(in ray sceneRay, in vec4 plane, in vec2 constraintsXY, in vec2 widthHeight, out vec2 uvCoord) {
	float planeIntersection = iPlane(sceneRay, plane);
	vec3 hitPoint = sceneRay.origin + sceneRay.direction*planeIntersection;
	if (hitPoint.z < constraintsXY.x || hitPoint.z > constraintsXY.x+widthHeight.x) {
		planeIntersection = -1.0;
	}
	else if (hitPoint.y < constraintsXY.y || hitPoint.y > constraintsXY.y+widthHeight.y) {
		planeIntersection = -1.0;
	}
	
	if (planeIntersection >= 0.0) {
		uvCoord = vec2(hitPoint.z - constraintsXY.x, hitPoint.y - constraintsXY.y);
	}
	else {
		uvCoord = vec2(-1.0);
	}
	return planeIntersection;
}

vec2 iSphere(in ray sceneRay, in vec4 sph) {
	//sphere at origin has equation |xyz| = r
	//so |xyz|^2 = r^2.
	//Since |xyz| = rayOrigin + t*rayDirection (where t is the distance to move along the ray),
	//we have rayOrigin^2 + 2*rayOrigin*t*rayDirection + t^2 - r^2. This is a quadratic equation, so:
	vec3 oc = sceneRay.origin - sph.xyz; //distance ray origin - sphere center
	
	float b = dot(oc, sceneRay.direction);
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
vec3 nSphere(in vec3 pos, in vec4 sph) {
	return normalize((pos - sph.xyz)/sph.w);
}

float fancyLight(ray primaryRay, vec4 light) {
	float luminance = 0.0;
	//vector from origin to light center
	vec3 originToLight = light.xyz - primaryRay.origin;
	
	//check to see if the light is behind us
	if(dot(primaryRay.direction, originToLight) >= 0.0) {
		//see if our ray is within the light boundary
		vec3 nearest = -(originToLight
						 + primaryRay.direction * dot(-originToLight, primaryRay.direction));
		float dist = length(nearest);
		if(dist <= light.w) {
			float lightness = (light.w - dist) / light.w;
			luminance = pow(lightness, 22.0);
		}
	}
	return luminance;
}

//check for ray intersection against scene elements
int intersect(in ray sceneRay, out vec2 t, out vec2 uvCoord) {
	t = vec2(1000.0);
	int hitId = -1;
	
	//check against the floor
	float tFloor = iPlane(sceneRay, sceneFloor);
	if (tFloor > 0.0 && tFloor < t.x) {
		t = vec2(tFloor);
		hitId = 0;
	}
	
	//check against the stained glass window
	float tWindow = iRectangle(sceneRay, sceneWindow, windowStart, windowWidthHeight, uvCoord);
	if (tWindow > 0.0 && tWindow < t.x) {
		t = vec2(tWindow);
		hitId = 1;
	}
	
	vec2 tSphere = iSphere(sceneRay, sceneSphere);
	if (tSphere.x > 0.0 && tSphere.x < t.x) {
		t = tSphere;
		hitId = 2;
	}
	
	return hitId;
}

vec4 doLighting(vec3 eyePoint, vec3 objPoint, vec3 normalAtPoint, vec3 lightPos, vec4 lightParams) {
	float fresnelBias = lightParams.x;
	float fresnelPower = lightParams.y;
	float fresnelScale = lightParams.z;
	float constAttenuation = 9000000.0;
	float linearAttenuation = 0.22;
	float quadraticAttenuation = 0.2;
	float dist = length(lightPos-objPoint);
	float attenuation = constAttenuation / ((1.0+linearAttenuation*dist)*(1.0+quadraticAttenuation*dist*dist));
	float shininess = lightParams.w;
	vec3 I = normalize(objPoint - eyePoint);
	vec3 lightDirection = normalize(lightPos-objPoint);
	vec3 viewDirection = normalize(eyePoint-objPoint);
	vec3 halfVector = normalize(lightDirection + viewDirection);
	float dif = clamp(dot(normalAtPoint, lightDirection), 0.0, 1.0);
	float spec = max(0.0, pow(dot(normalAtPoint, halfVector), shininess));
	float fresnel = clamp(fresnelBias + fresnelScale * pow(1.0 + dot(I, normalAtPoint), fresnelPower), 0.0, 1.0);
	return attenuation * vec4(vec3(mix(spec, dif, fresnel)), 1.0);
}

vec4 doLighting(vec3 eyePoint, vec3 objPoint, vec3 normalAtPoint, vec3 lightPos) {
	float fresnelBias = 0.25;
	float fresnelPower = 5.0;
	float fresnelScale = 1.0;
	float shininess = 20.0;
	vec4 lightParams = vec4(fresnelBias, fresnelPower, fresnelScale, shininess);
	return doLighting(eyePoint, objPoint, normalAtPoint, lightPos, lightParams); 
}

void doShadowColor(in ray primaryRay, inout vec4 col) {
	vec4 returnColor = vec4(0.0);
	vec2 shadowUV;
	vec2 shadowT;
	int shadowId;
	float shadowCheckDelta = light.w;
	ray shadowRay;
	shadowRay.lightColor = primaryRay.lightColor;
	shadowRay.transmittance = primaryRay.transmittance;
	vec3 pos = primaryRay.origin + primaryRay.rayLength*primaryRay.direction;
	shadowRay.origin = pos - 0.001*primaryRay.direction;
	for (int i = 0; i < 7; i++) {
		//soft shadows
		if (i == 0) {
			vec4 tempLight = light;
			tempLight.x += shadowCheckDelta;
			shadowRay.direction = normalize(tempLight.xyz-pos);
		}
		else if (i == 1) {
			vec4 tempLight = light;
			tempLight.x -= shadowCheckDelta;
			shadowRay.direction = normalize(tempLight.xyz-pos);
		}
		else if (i == 2) {
			vec4 tempLight = light;
			tempLight.y += shadowCheckDelta;
			shadowRay.direction = normalize(tempLight.xyz-pos);
		}
		else if (i == 3) {
			vec4 tempLight = light;
			tempLight.y -= shadowCheckDelta;
			shadowRay.direction = normalize(tempLight.xyz-pos);
		}
		else if (i == 4) {
			vec4 tempLight = light;
			tempLight.z += shadowCheckDelta;
			shadowRay.direction = normalize(tempLight.xyz-pos);
		}
		else if (i == 5) {
			vec4 tempLight = light;
			tempLight.z -= shadowCheckDelta;
			shadowRay.direction = normalize(tempLight.xyz-pos);
		}
		else
			shadowRay.direction = normalize(light.xyz-pos);
		shadowId = intersect(shadowRay, shadowT, shadowUV);
		vec3 shadowHit = shadowRay.origin + shadowT.x * shadowRay.direction;
		
		//if we have a non-negative id, we've hit something
		if (shadowId >= 0 && primaryRay.lastHitObject >= 0) {
			vec4 tempColor;
			if (light.y > 0.0) {
				if (primaryRay.lastHitObject != 1
					&& shadowId == 1
					&& length(light.xyz-shadowRay.origin) > length(shadowHit-shadowRay.origin)) {
					//shade objects that are shadowed by the window
					vec3 nor = sceneWindow.xyz;
					shadowRay.lightColor = doLighting(primaryRay.origin, shadowHit, nor, light.xyz);
					shadowRay.lightColor *= 1.0 - vec4(Voronoi(shadowUV),1.0);
					shadowRay.transmittance = primaryRay.transmittance * glassTransmission;
					tempColor = mix(shadowRay.lightColor, col, shadowRay.transmittance);
				}
				else if (primaryRay.lastHitObject == 1) {
					//shade the back side of the window
					vec3 nor = -sceneWindow.xyz;
					shadowRay.lightColor = doLighting(primaryRay.origin, shadowHit, nor, light.xyz);
					shadowRay.lightColor *= vec4(Voronoi(shadowUV),1.0);
					shadowRay.transmittance = primaryRay.transmittance * glassTransmission;
					tempColor = mix(shadowRay.lightColor, col, shadowRay.transmittance);
				}
				if (primaryRay.lastHitObject != 1 && shadowId != 1) {
					//shadows for everything else in the scene
					shadowRay.lightColor = shadowRay.lightColor;
					shadowRay.transmittance = 0.5*primaryRay.transmittance;
					tempColor = mix(shadowRay.lightColor, col, 1.0-shadowRay.transmittance);
				}
			}
			else if (primaryRay.lastHitObject >= 0) {
				//before "sunrise"
				shadowRay.lightColor = shadowRay.lightColor;
				shadowRay.transmittance = 0.5*primaryRay.transmittance;
				tempColor = mix(shadowRay.lightColor, col, shadowRay.transmittance);
			}
			returnColor += tempColor;
		}
	}
	//if we use a number slightly higher than our iteration count,
	//then we get dark, but not black, shadows.  This also washes
	//out the color of the color of the glass, so it's kind of a
	//trade-off.
	col -= returnColor*(1.0/8.5);
}

void main(void)
{
	//pixel coordinates from 0 to 1
	float aspectRatio = iResolution.x/iResolution.y;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	
	float time = mod(0.1*iGlobalTime, M_PI);
	float sinTime = sin(time);
	light.y = light.y + 500.0 * sinTime-100.0;
	light.z = 450.0 * cos(time);
	
	vec2 mouse0To2PI = iMouse.xy/iResolution.xy*2.0*M_PI;
	mat4 cameraRotation = rotateY(mouse0To2PI.x);
	vec4 ro = vec4(startingLocation, 1.0);
	mat4 compound = translate(-ro.xyz) * cameraRotation * translate(ro.xyz);
	ro = compound * ro;
	
	//generate a ray with origin ro and direction rd
	ray primaryRay;
	primaryRay.origin = ro.xyz;
	primaryRay.direction = (cameraRotation * vec4(normalize(vec3( (-1.0+2.0*uv) * vec2(aspectRatio, 1.0), -1.0)), 1.0)).xyz;
	primaryRay.lightColor = vec4(1.0);
	primaryRay.transmittance = 1.0;
	primaryRay.rayLength = -1.0;
	primaryRay.lastHitObject = -1;
	
	ray traceRay = primaryRay;
	
	
	vec4 col = vec4(1.0);
	//intersect the ray with scene
	vec2 uvCoord;
	vec2 t;
	//did we hit something?
	for (int i = 0; i < 8; i++) {
		int id = intersect(traceRay, t, uvCoord);
		if (traceRay.transmittance > 0.01) {
			traceRay.lastHitObject = id;
			if (primaryRay.lastHitObject < 0)
				primaryRay.lastHitObject = traceRay.lastHitObject;
			//find the point where we hit
			traceRay.rayLength = t.x;
			if (primaryRay.rayLength < 0.0)
				primaryRay.rayLength = traceRay.rayLength;
			
			vec3 pos = traceRay.origin + traceRay.rayLength*traceRay.direction;
			traceRay.origin = pos + 0.001 * traceRay.direction;
			
			if (id == 0) {
				col = doLighting(primaryRay.origin, pos, sceneFloor.xyz, light.xyz);
				traceRay.direction = reflect(traceRay.direction, sceneFloor.xyz);
				traceRay.transmittance *= floorTransmission;
				traceRay.lightColor = col;
				vec4 groundTextureColor = texture2D(iChannel1, 0.125*pos.xz, 0.0);
				col = mix(primaryRay.lightColor, col, traceRay.transmittance);
				col *= vec4(groundTextureColor.rgb, 1.0);
			}
			else if (id == 1) {
				traceRay.lightColor = vec4(Voronoi(uvCoord),1.0);
				traceRay.transmittance *= glassTransmission;
				ray glassRay = traceRay;
				traceRay.direction = reflect(traceRay.direction, sceneWindow.xyz);
				//did we hit something after the glass?
				int id = intersect(glassRay, t, uvCoord);
				if (id != 1 && glassRay.transmittance > 0.0) {
					glassRay.lastHitObject = id;
					glassRay.rayLength = t.x;
					
					//do stained glass coloring on the other side... effectively another shadow call
					vec4 stainedShadowColor = col;
					//This appears to break in Firefox, but not Chrome:
					//doShadowColor(glassRay, stainedShadowColor);
					vec3 pos = glassRay.origin + glassRay.rayLength*glassRay.direction;
					
					if (id == 0) {
						vec4 groundTextureColor = texture2D( iChannel1, 0.125*pos.xz, 0.0 );
						col = mix(groundTextureColor, col, 1.0-glassRay.transmittance);
					}
					vec4 lighting = 0.25*doLighting(primaryRay.origin, pos, sceneFloor.xyz, light.xyz);
					col = mix(lighting, col, 1.0-glassRay.transmittance);
					
					col = mix(stainedShadowColor, col, 1.0-glassRay.transmittance);
					
					col = mix(glassRay.lightColor, col, 1.0-glassRay.transmittance);
				}
			}
			else if (id == 2) {
				vec3 sphNormal = nSphere(pos, sceneSphere);
				traceRay.direction = reflect(traceRay.direction, sphNormal);
				traceRay.transmittance *= sphereTransmission;
				float fresnelBias = 0.25;
				float fresnelPower = 5.0;
				float fresnelScale = 1.0;
				float shininess = 5.0;
				vec4 lightParams = vec4(fresnelBias, fresnelPower, fresnelScale, shininess);
				col = doLighting(primaryRay.origin, pos, sphNormal, light.xyz, lightParams);
				traceRay.lightColor = col;
				col = mix(traceRay.lightColor, col, traceRay.transmittance);
			}
			else {
				//sky
				col = vec4(0.7*sinTime, 0.65*sinTime, sinTime, 1.0);
				traceRay.lightColor = col;
			}
		}
	}
	if (primaryRay.lastHitObject < 0 || primaryRay.lastHitObject == 1) {
		if (light.y > 0.0) {
			float lightValue = 2.0*fancyLight(primaryRay, light);
			col += vec4(vec3(1.0-col.rgb)*lightValue*primaryRay.lightColor.rgb, 1.0);
		}
	}
	
	doShadowColor(primaryRay, col);
	
	gl_FragColor = clamp(col, vec4(0.0), vec4(1.0));
}
