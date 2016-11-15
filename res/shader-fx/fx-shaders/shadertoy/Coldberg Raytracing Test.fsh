// https://www.shadertoy.com/view/XdfXW8

// iChannel0: t0
// iChannel1: t0
// iChannel2: c0

#define PI 				3.14159265359
#define EPSILON			1.0e-6

#define DRAW_DISTANCE	100000.0
#define BOUNCE_COUNT 	4
#define OBJECT_SKY		0
#define OBJECT_SPHERE 	1
#define OBJECT_FLOOR	2
#define OBJECT_LIGHTA	3
#define OBJECT_LIGHTB	4
#define SPREAD_FACTOR   0.001
#define SAMPLE_COUNT	6

	

void set_color (vec3 c) {
	gl_FragColor = vec4 (c, 1.0);
}
vec3 texture3d (vec3 uv, vec3 normal, float scale, sampler2D tex) {
	return (
		texture2D (tex, uv.xy * scale) * abs (normal.z) +
		texture2D (tex, uv.xz * scale) * abs (normal.y) +
		texture2D (tex, uv.yz * scale) * abs (normal.x)).xyz ;	
}


vec3 skybox_texture (vec3 vRayDirection) {
	return textureCube (iChannel2,vRayDirection).xyz;
}

vec3 cartesian_to_spherical (vec3 vPoint) {
	float r = length (vPoint);
	return vec3(
		(acos (vPoint.z / r) + PI) / (2.0 * PI),
		(atan (vPoint.y, vPoint.x) + PI) / (2.0 * PI),
		r
	);
}

mat3 rotate_x (float fi) {
	float cfi = cos (fi);
	float sfi = sin (fi);
	return mat3 (
		vec3 (1.0, 0.0, 0.0),
		vec3 (0.0, cfi, -sfi),
		vec3 (0.0, sfi, cfi)
	);
}

mat3 rotate_y (float fi) {
	float cfi = cos (fi);
	float sfi = sin (fi);
	return mat3 (
		vec3 (cfi, 0.0, sfi),
		vec3 (0.0, 1.0, 0.0),
		vec3 (-sfi, 0.0, cfi)
	);
}

mat3 rotate_z (float fi) {
	float cfi = cos (fi);
	float sfi = sin (fi);
	return mat3 (
		vec3 (cfi, -sfi, 0.0),
		vec3 (sfi, cfi, 0.0),
		vec3 (0.0, 0.0, 1.0)
	);
}

mat3 rotate (vec3 vPitchYawRoll, float fi) {
	return
		rotate_x (fi * vPitchYawRoll.x) *
		rotate_y (fi * vPitchYawRoll.y) *
		rotate_z (fi * vPitchYawRoll.z) ;		
}


mat4 orientation (vec3 vNormal, vec3 vTangent) {
	vec3 vBitangent = -cross (vNormal, vTangent);
	return mat4 (
		vec4 (vBitangent, 	0.0),
		vec4 (vNormal, 		0.0),
		vec4 (vTangent, 	0.0),
		vec4 (0.0, 0.0, 0.0, 1.0));
}

void setup_camera_ray (
	in vec3 vEyeOrigin,
	in vec3 vEyeDirection,
	in vec3 vEyeUpward,
	in float qFieldOfView,
	out vec3 vRayOrigin,
	out vec3 vRayDirection)
{
	vec2 vScreen = 
		vec2 (gl_FragCoord.xy*2.0 - iResolution.xy) / 
		vec2 (min (iResolution.x, iResolution.y));
		
	mat4 mCamera = orientation (vEyeUpward, vEyeDirection);	
	float tAlpha = tan ((qFieldOfView * PI) / 360.0);
	vRayDirection = normalize (vec4 (vScreen * tAlpha, 1.0, 1.0) * mCamera).xyz ;
	vRayOrigin = vEyeOrigin;
}

void look_at_point (
	in vec3 vPointOfInterest,
	in vec3 vPointOfView,
	in float qViewRoll,
	out vec3 vOrigin,
	out vec3 vDirection,
	out vec3 vUpward)
{
	vOrigin = vPointOfView;
	vDirection = normalize (vPointOfInterest - vPointOfView);
		
	vec3 vInward = vDirection * vec3 (1.0, 0.0, 1.0);
	vec3 vLeft = normalize (vInward * rotate_y (PI/2.0));
	vUpward = cross (vDirection, vLeft);	
}

float rand (vec2 co){
  	return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

vec3 jitter (vec3 co) {
	return vec3 (
		rand (co.xy*iGlobalTime),
		rand (co.yz*iGlobalTime),
		rand (co.xz*iGlobalTime));
}

bool ray_plane_intersect (
	in vec3 vRayOrigin,
	in vec3 vRayDirection,
	in vec3 vPlaneOrigin, 
	in vec3 vPlaneNormal,
	out float qDistance,
	out vec3 vIntersectPoint,
	out vec3 vProjectedPoint,
	out vec3 vNormal)
{
	float qPlaneRay = dot (vPlaneNormal, vRayDirection);
	if (qPlaneRay <= EPSILON) {
		return false;
	}
	qDistance = dot (vPlaneOrigin - vRayOrigin, vPlaneNormal) / qPlaneRay;
	vIntersectPoint = vRayOrigin + vRayDirection * qDistance;	
	vProjectedPoint = vIntersectPoint;
	vNormal = vPlaneNormal;
	return true;
}

bool ray_sphere_intersect (
	in vec3 vRayOrigin, 
	in vec3 vRayDirection, 
	in vec3 vSphereOrigin, 
	in vec3 vSphereNormal,
	out float qDistance,
	out vec3 vIntersectPoint,
	out vec3 vProjectedPoint,
	out vec3 vNormal) 
{
	vRayOrigin = vRayOrigin - vSphereOrigin;
	float r = length (vSphereNormal);
	float a = dot (vRayDirection, vRayDirection);
	float b = dot (2.0 * vRayDirection, vRayOrigin);
	float c = dot (vRayOrigin, vRayOrigin) - pow (r,2.0);
	float disc = b * b - 4.0 * a * c;
	if (disc < 0.0)
		return false;
	float distSqrt = sqrt (disc);
	float q = b < 0.0 ? 
		(-b - distSqrt)/2.0 :
		(-b + distSqrt)/2.0 ;
	float t0 = min (q / a, c / q);
	float t1 = max (q / a, c / q);
	if (t1 < 0.0)
		return false;
	qDistance = t0 < 0.0 ? t1 : t0 ;
	vIntersectPoint = vRayOrigin + vRayDirection * qDistance;
	vProjectedPoint = vIntersectPoint;
	vNormal = -normalize (vIntersectPoint - vSphereOrigin) ;
	return true;
}


vec3 vLightAOrigin = vec3 (-10.0, 9.0, 1.0) * 
	 rotate (vec3 (1.0, 1.0, 0.0), iGlobalTime * PI / 4.0) + vec3 (5.0, 10.0, 10.0);

vec3 vLightBOrigin = vec3 (-10.0, 9.0, 1.0) * 
	 rotate (vec3 (1.0, 1.0, 0.0), iGlobalTime * PI / 4.0 + PI/32.0) + vec3 (5.0, 10.0, 10.0);

const vec3 vLightANormal = vec3 (0.1, 0.0, 0.0) ;
const vec3 vLightBNormal = vec3 (0.1, 0.0, 0.0) ;

const vec3 vLightBColor = vec3 (1.0, 0.5, 0.0);
const vec3 vLightAColor = vec3 (0.0, 0.5, 1.0);


int intersect_object (
	in vec3 vRayOrigin,
	in vec3 vRayDirection,
	out float qMinDistance,
	out vec3 oPoint,
	out vec3 oProjected,
	out vec3 oNormal)
{
	const vec3 vFloorOrigin = vec3 (1.0, 0.0, 0.0) ;
	const vec3 vFloorNormal = vec3 (0.0, -1.0, 0.0); 
	const vec3 vWallOrigin = vec3 (-20.0, 0.0, -10.0) ;
	const vec3 vWallNormal = vec3 (0.0, 0.0, -1.0); 
	
	const vec3 vSphereOrigin = vec3 (0.0, 5.0, 0.0) ;
	const vec3 vSphereNormal = vec3 (2.5, 0.0, 0.0) ;
		
	float qDistance = 0.0;
	vec3 vPoint 	= vec3 (0.0);
	vec3 vProjected = vec3 (0.0);
	vec3 vNormal 	= vec3 (0.0);
	int iObjectID 	= OBJECT_SKY;
	
	 qMinDistance = DRAW_DISTANCE;
	
	oPoint = vRayDirection * DRAW_DISTANCE;
	oNormal = -vRayDirection;
	oProjected = vRayDirection;	

		
	if (ray_sphere_intersect (
		vRayOrigin,
		vRayDirection,
		vSphereOrigin * (sin (iGlobalTime) + 1.5),
		vSphereNormal,
		qDistance,
		vPoint,
		vProjected,
		vNormal))
	{
		if (qDistance < qMinDistance) {
			oPoint = vPoint;
			oProjected = vProjected;
			oNormal = vNormal;
			iObjectID = OBJECT_SPHERE;
			qMinDistance = qDistance;
		}
	}
	
	if (ray_plane_intersect (
		vRayOrigin, 
		vRayDirection, 
		vFloorOrigin, 
		vFloorNormal,
		qDistance,
		vPoint,
		vProjected,
		vNormal)) 
	{
		if (qDistance < qMinDistance) {
			oPoint = vPoint;
			oProjected = vProjected;
			oNormal = vNormal;
			iObjectID = OBJECT_FLOOR;
			qMinDistance = qDistance;
		}
	}
	
	if (ray_plane_intersect (
		vRayOrigin, 
		vRayDirection, 
		vWallOrigin, 
		vWallNormal,
		qDistance,
		vPoint,
		vProjected,
		vNormal)) 
	{
		if (qDistance < qMinDistance) {
			oPoint = vPoint;
			oProjected = vProjected;
			oNormal = vNormal;
			iObjectID = OBJECT_FLOOR;
			qMinDistance = qDistance;
		}
	}
	
	if (ray_sphere_intersect (
		vRayOrigin,
		vRayDirection,
		vLightAOrigin,
		vLightANormal,
		qDistance,
		vPoint,
		vProjected,
		vNormal))
	{
		if (qDistance < qMinDistance) {
			oPoint = vPoint;
			oProjected = vProjected;
			oNormal = vNormal;
			iObjectID = OBJECT_LIGHTA;
			qMinDistance = qDistance;
		}
	}
	
	if (ray_sphere_intersect (
		vRayOrigin,
		vRayDirection,
		vLightBOrigin,
		vLightBNormal,
		qDistance,
		vPoint,
		vProjected,
		vNormal))
	{
		if (qDistance < qMinDistance) {
			oPoint = vPoint;
			oProjected = vProjected;
			oNormal = vNormal;
			iObjectID = OBJECT_LIGHTB;
			qMinDistance = qDistance;
		}
	}

	
	return iObjectID;
}

vec3 get_material (
	in int iObject, 
	in vec3 vRayDirection,
	in vec3 vProjected,
	in vec3 vNormal,
	out float qReflectiveness)
{
	if (iObject == OBJECT_SKY) {
		qReflectiveness = 0.0;
		return skybox_texture (vRayDirection);
	}		
	else if (iObject == OBJECT_SPHERE) {
		qReflectiveness = 0.7;
		return texture3d (vProjected, vNormal, 0.1, iChannel1);
	}
	else if (iObject == OBJECT_FLOOR) {
		qReflectiveness = 0.2;
		return texture3d (vProjected, vNormal, 0.1, iChannel0);
	}	
	else if (iObject == OBJECT_LIGHTA) {
		qReflectiveness = 0.0;
		return vLightAColor;	
	}
	else if (iObject == OBJECT_LIGHTB) {
		qReflectiveness = 0.0;
		return vLightBColor;
	}
	
	
	qReflectiveness = 0.0;
	return skybox_texture (vRayDirection);	
}

vec3 get_lighting (vec3 vRayOrigin, vec3 vPointNormal) {
	vec3 vPoint 		= vec3 (0.0);
	vec3 vNormal 		= vec3 (0.0);
	vec3 vProjected 	= vec3 (0.0);
	vec3 vLight			= vec3 (0.0);
	vec3 vRayDirection  = vec3 (0.0);
	float qDistance		= 0.0;	
	int iObject = 0;
	
	vRayDirection =-normalize (vRayOrigin - vLightAOrigin);
	iObject = intersect_object (
		vRayOrigin, 
		vRayDirection,
		qDistance,
		vPoint,
		vProjected,
		vNormal);
	if (iObject == OBJECT_SKY || iObject == OBJECT_LIGHTA || iObject == OBJECT_LIGHTB)
		vLight += vLightAColor * dot (-vPointNormal, vRayDirection) * pow (0.96, qDistance);
	
	vRayDirection =-normalize (vRayOrigin - vLightBOrigin);
	iObject = intersect_object (
		vRayOrigin, 
		vRayDirection,
		qDistance,
		vPoint,
		vProjected,
		vNormal);
	if (iObject == OBJECT_SKY || iObject == OBJECT_LIGHTA || iObject == OBJECT_LIGHTB)
		vLight += vLightBColor * dot (-vPointNormal, vRayDirection) * pow (0.96, qDistance);
	
	return vLight;
}


vec3 trace_ray (
	in vec3 vRayOrigin,
	in vec3 vRayDirection)
{
	float qDistance			= 0.0;
	
	vec3 vPoint 			= vec3 (0.0);
	vec3 vNormal 			= vec3 (0.0);
	vec3 vProjected		 	= vec3 (0.0);
	vec3 vColor 			= vec3 (0.0);
	vec3 vLight 			= vec3 (1.0);
	for (int i = 0;i < SAMPLE_COUNT; ++i) {	
		int iObject = intersect_object (
			vRayOrigin, 
			vRayDirection + jitter (vec3 (float(i) * vRayDirection))*SPREAD_FACTOR,
			qDistance,
			vPoint,
			vProjected,
			vNormal);
		float q = 0.0;
		vec3 mColor = get_material (iObject, vRayDirection, vProjected, vNormal, q);		
		vec3 vLight = get_lighting (vPoint, normalize(vNormal));
		vColor += (mColor * vLight) ;
	}
	return vColor / float (SAMPLE_COUNT);
}



void main (void) {
	const float qFieldOfView = 60.0; 
	
	mat3 vRotation  = rotate (vec3 (0.0, 1.0, 0.0), iGlobalTime * PI / 4.0) ;
	
	vec3 vEyeOrigin 	= vec3 (0.0, 15.0,20.0);
	vec3 vEyeDirection 	= vec3 (0.0, 0.0, -1.0) * rotate_x (-PI/8.0);
	vec3 vEyeUpward 	= vec3 (0.0, 1.0, 0.0) * rotate_x (-PI/8.0);
	/*
	look_at_point (
		vec3 (0.0, 0.0, 0.0), 
		vec3 (0.0, 20.0, 10.0) * vRotation,
		PI * 2.0,
		vEyeOrigin,
		vEyeDirection,
		vEyeUpward);
	*/			
	vec3 vRayOrigin 	= vec3 (0.0);
	vec3 vRayDirection 	= vec3 (0.0);
	
	setup_camera_ray (
		vEyeOrigin, 
		vEyeDirection, 
		vEyeUpward,
		qFieldOfView,
		vRayOrigin,
		vRayDirection
	);
	
	set_color (trace_ray (vRayOrigin, vRayDirection));	
}
