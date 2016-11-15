// https://www.shadertoy.com/view/4sj3WG

// iChannel0: t1
// iChannel1: t13
// iChannel2: t3
// iChannel3: t2

//==RAY MARCHING CONSTANTS=========================================================
#define EPSILON .001
#define MAX_VIEW_STEPS 64
#define MAX_SHADOW_STEPS 128
#define OCCLUSION_SAMPLES 8.0
#define OCCLUSION_FACTOR 1.667
#define MAX_DEPTH 2.0
#define BUMP_FACTOR .025
#define TEX_SCALE_FACTOR 1.2
#define BUMP_SCALE_FACTOR .8

//==SCENE OBJECT CONSTANTS=========================================================
#define CAM_POS vec3(0.0, 0.8, -0.4)
#define CAM_DIR vec2(0.1, 1.0)
#define CAM_BACK 0.5
#define CAM_ZOOM 1.0

#define ROOM_POS vec3 (0.0, 0.8, 0.0)
#define ROOM_DIM vec3 (1.5, 0.8, 1.5)

#define STAIR_POS vec3(1.5, 0.8, 0.0)
#define STAIR_DIM vec3(.8, .01, .05)
#define STAIR_REPEAT vec3(0.0, -.08, .125)

#define PLANE_NORM vec3(0.0, 1.0, 0.0)
#define PLANE_HEIGHT 0.0

#define AMBIENT .01

#define LIGHT_POS_A vec3(1.0, 0.05, -0.75)
#define LIGHT_DIR_A vec3(0.5, 0.7, 0.86)
#define LIGHT_COLOR_A vec4(1.0, 0.33, 0.0, 1.0)
#define LIGHT_BRIGHT_A texture2D(iChannel2, vec2(iGlobalTime+.25)).g*1.5+.5
#define LIGHT_SPREAD_A .15
#define LIGHT_PENUMBRA_A 16.0

#define LIGHT_POS_B vec3(1.0, 0.8, 1.4)
#define LIGHT_DIR_B vec3(0.5, 0.0, -0.86)
#define LIGHT_COLOR_B vec4(.25, .25, .94, 1.0)
#define LIGHT_BRIGHT_B texture2D(iChannel2, vec2(iGlobalTime)).r*2.0+1.0
#define LIGHT_SPREAD_B .15
#define LIGHT_PENUMBRA_B 16.0

#define WALL_BUMP iChannel2
#define CEIL_BUMP iChannel1
#define GROUND_BUMP iChannel1
#define WALL_TEX iChannel0
#define GROUND_TEX iChannel2
#define CEIL_TEX iChannel2
#define STAIR_TEX iChannel3

//==RENDERING STRUCTURES===========================================================

/*
	A structure for a spotlight.
*/
struct SpotLight
{
	vec3 position, direction;
	vec4 color;
	float brightness;
	float spread;
	float penumbraFactor;
} under, onlooking;

//==CAMERA FUNCTIONS================================================================
/*
	TEKF https://www.shadertoy.com/view/XdsGDB !!!
	Set up a camera looking at the scene.
	origin - camera is positioned relative to, and looking at, this point
	dist - how far camera is from origin
	rotation - about x & y axes, by left-hand screw rule, relative to camera looking along +z
	zoom - the relative length of the lens
*/
void camPolar( out vec3 pos, out vec3 dir, in vec3 origin, in vec2 rotation, in float dist, in float zoom )
{
	// get rotation coefficients
	vec2 c = cos(rotation);
	vec4 s;
	s.xy = sin(rotation);
	s.zw = -s.xy;

	// ray in view space
	dir.xy = gl_FragCoord.xy - iResolution.xy*.5;
	dir.z = iResolution.y*zoom;
	dir = normalize(dir);
	
	// rotate ray
	dir.yz = dir.yz*c.x + dir.zy*s.zx;
	dir.xz = dir.xz*c.y + dir.zx*s.yw;
	
	// position camera
	pos = origin - dist*vec3(c.x*s.y,s.z,c.x*c.y);
}

//==TEXTURING FUNCTIONS=============================================================
/*
	By Reinder. Takes a 3D coordinate, and returns a texel based on which plane(s)
	it lies in.
*/
vec4 tex3D( in vec3 pos, in vec3 normal, sampler2D sampler )
{
	return 	texture2D( sampler, pos.yz )*abs(normal.x)+ 
			texture2D( sampler, pos.zx )*abs(normal.y)+ 
			texture2D( sampler, pos.xy )*abs(normal.z);
}
/*
	By Reinder. Takes a 3D coordinate, and returns a texel based on which plane(s)
	it lies in.
*/
vec4 texRoom( in vec3 pos, in vec3 normal, 
			 sampler2D wall, sampler2D ground, sampler2D ceiling )
{
	pos *= TEX_SCALE_FACTOR; 
	return 	texture2D( wall, pos.yz )*abs(normal.x)+
			((normal.y < 0.0)?texture2D( ceiling, pos.zx ):texture2D( ground, pos.zx ))*abs(normal.y)+
			texture2D( wall, pos.xy )*abs(normal.z);
}

//==DISTANCE FUNCTIONS==============================================================
/*
	Returns the distance to the surface of a box with the given position and dimensions.
	This also returns a signed distance. If the returned value is negative, you are within
	the box.
*/
float distRoom(vec3 samplePos, vec3 boxPos, vec3 boxDim)
{
	vec3 d = abs(samplePos-boxPos) - boxDim;
	return -(min(max(d.x,max(d.y,d.z)),0.0) +
		length(max(d,0.0)));
}

/*
	Constructs a distance field for the staircase by repeating a box and ceiling its y
	value based on its z location..
*/
float distStairs(vec3 samplePos, vec3 stairPos, vec3 stairDim)
{
	samplePos.z += STAIR_REPEAT.z*.5;
	samplePos.y -= ceil(samplePos.z/STAIR_REPEAT.z)*STAIR_REPEAT.y;
	samplePos.z = mod(samplePos.z, STAIR_REPEAT.z);
	samplePos.z -= STAIR_REPEAT.z*.5;
	
	return length(max(abs(stairPos-samplePos)-stairDim,0.0));
}

/*
	Returns the distance to the surface of a box, but a box that's been modified
	by a bump map.
*/
float distRoomBump(vec3 samplePos, vec3 boxPos, vec3 boxDim, 
				   sampler2D wall, sampler2D ground, sampler2D ceiling)
{	
	vec3 normal;
	float bump = 0.0;
	if(length(samplePos-boxPos) < length(boxDim))
	{
	   	normal = normalize(samplePos-boxPos);
 		bump = texRoom(samplePos*BUMP_SCALE_FACTOR, normal, wall, ground, ceiling).r*BUMP_FACTOR;
	}
	vec3 d = abs(samplePos-boxPos) - boxDim;
	return -(min(max(d.x,max(d.y,d.z)),0.0) +
		length(max(d,0.0))+bump);
}

float getDist(vec3 samplePos)
{
	return min(distRoomBump(samplePos, ROOM_POS, ROOM_DIM, WALL_BUMP, GROUND_BUMP, CEIL_BUMP),
			   distStairs(samplePos, STAIR_POS, STAIR_DIM));
}

//==RAY MARCHING FUNCTIONS=========================================================
/*
		Marches the 3D point <pos> along the given direction.
	When the point is either stepped the maximum number of times,
	has passed the maximum distance, or is within a set distance
	from geometry the function returns. 
		Note that the position is passed by reference and is modified
	for use within the function.
*/
void marchThroughField(inout vec3 pos, vec3 dir)
{
	float dist;
	for(int i = 0; i < MAX_VIEW_STEPS; i++)
	{
		dist = getDist(pos);
		if(dist < EPSILON || dist > MAX_DEPTH)
			return;
		else	
			pos += dir*dist;
	}
	return;
}

//==LIGHT CALCULATION FUNCTIONS=====================================================
/*
	Returns the surface normal of a point in the distance function.
*/
vec3 getNormal(vec3 pos)
{
	float d=getDist(pos);
	return normalize(vec3( getDist(pos+vec3(EPSILON,0,0))-d, getDist(pos+vec3(0,EPSILON,0))-d, getDist(pos+vec3(0,0,EPSILON))-d ));
}

/*
	Calculates the ambient occlusion factor at a given point in space.
*/
float calcOcclusion(vec3 pos, vec3 surfaceNormal)
{
	float result = 0.0;
	vec3 normalPos = vec3(pos);
	for(float i = 0.0; i < OCCLUSION_SAMPLES; i+=1.0)
	{
		normalPos += surfaceNormal * (1.0/OCCLUSION_SAMPLES);
		result += (1.0/exp2(i)) * (i/OCCLUSION_SAMPLES)-getDist(normalPos);
	}
	return 1.0-(OCCLUSION_FACTOR*result);
}

/*
	Calculates how much light remains if shadows are considered.
*/
float calcShadow( vec3 origin, vec3 lightDir, SpotLight light)
{
	float dist;
	float result = 1.0;
	float lightDist = length(light.position-origin);
	
	vec3 pos = vec3(origin)+(lightDir*EPSILON*10.0);
	
	for(int i = 0; i < MAX_SHADOW_STEPS; i++)
	{
		dist = getDist(pos);
		if(dist < EPSILON)
		{
			return 0.0;
		}
		if(length(pos-origin) > lightDist || length(pos-origin) > MAX_DEPTH)
		{
			return result;
		}
		pos+=lightDir*dist;
		if( length(pos-origin) < lightDist )
		{
			result = min( result, light.penumbraFactor*dist / length(pos-origin) );
		}
	}
	return result;
}

//IQ fog
vec4 applyFog( in vec4  rgba,      // original color of the pixel
               in float dist, // camera to point distance
               in vec3  rayOri,   // camera position
               in vec3  rayDir )  // camera to point vector
{
	float b = 3.0, c = .4;
    float fogAmount = c * exp(-rayOri.y*b) * (1.0-exp( -dist*rayDir.y*b ))/rayDir.y;
    vec4  fogColor  = vec4(0.005, 0.005, 0.01, 1.0);
    return mix( rgba, fogColor, fogAmount );
}

/*
	Returns the product of the Phong lighting equation on a point in space given
	a (SPOT) light and the surface's material.
*/
vec4 calcLighting(vec3 samplePos, vec3 eyeDir, vec3 normal, vec3 reflection, SpotLight light)
{
	
	float lightDist = length(light.position-samplePos);
	vec3 lightDir = normalize(light.position-samplePos);
	
	float specular = 0.0, diffuse = 0.0, ambient = AMBIENT;
	float attenuation = 0.0, shadow = 0.0, occlusion = 0.0;
	
	float spotCos = dot(-lightDir, light.direction);
	float spotCoefficient = smoothstep( 1.0-light.spread, 1.0, spotCos );
	
	// If it's outside of the light's cone we don't need to calculate any terms.
	if(spotCos > 1.0-light.spread)
	{
		shadow = calcShadow(samplePos, lightDir, light);
		if(shadow >= EPSILON)
		{
			specular = pow(max( 0.0, dot(lightDir, reflection)*spotCoefficient), 2000.0);
			diffuse = max( 0.0, dot(lightDir, normal)*spotCoefficient);
			attenuation = min(1.0, (1.0/(lightDist/light.brightness)));
		}
		//else occlusion = calcOcclusion(samplePos, normal);
	}
	occlusion = calcOcclusion(samplePos, normal);
	
	return light.color*clamp(((specular+diffuse)*shadow*attenuation), 0.0, 1.0)+(ambient*occlusion);
}

vec4 calcLights(vec3 samplePos, vec3 eye, vec3 normal, SpotLight a, SpotLight b, vec4 texel)
{
	vec3 eyeDir = normalize(samplePos-eye);
	vec3 reflection = normalize(reflect(eyeDir, normal));
	vec4 lightingA = calcLighting(samplePos, eyeDir, normal, reflection, a);
	vec4 lightingB = calcLighting(samplePos, eyeDir, normal, reflection, b);
	return min(vec4(1.0), (lightingA+lightingB))*texel;
}
	

/*
	Determines the texture of the current surface position.
*/
vec4 getSurfaceTexel(vec3 pos, vec3 normal)
{
	if(distRoom(pos, ROOM_POS, ROOM_DIM) <= distStairs(pos, STAIR_POS, STAIR_DIM))
	{
		return texRoom(pos, normal, WALL_TEX, GROUND_TEX, CEIL_TEX);
	}
	else
	{
		return tex3D(pos, normal, STAIR_TEX);
	}
}

vec4 shade(vec3 pos, vec3 dir, vec3 eye, SpotLight a, SpotLight b)
{                                          
	vec3 normal = getNormal(pos);
	vec4 texel = getSurfaceTexel(pos, normal);
	vec4 color = calcLights(pos, eye, normal, a, b, texel);
	return applyFog(color, length(pos-eye), eye, dir);
}

void main(void)
{
	vec3 pos, eye, dir;
	camPolar(pos, dir, CAM_POS, CAM_DIR, CAM_BACK, CAM_ZOOM);
	eye = vec3(pos);
	
	under = SpotLight(LIGHT_POS_A, normalize(LIGHT_DIR_A), LIGHT_COLOR_A, 
					  LIGHT_BRIGHT_A, LIGHT_SPREAD_A, LIGHT_PENUMBRA_A);
	onlooking = SpotLight(LIGHT_POS_B, normalize(LIGHT_DIR_B), LIGHT_COLOR_B, 
					  LIGHT_BRIGHT_B, LIGHT_SPREAD_B, LIGHT_PENUMBRA_B);
	
	
	marchThroughField(pos, dir);
	gl_FragColor = shade(pos, dir, eye, under, onlooking);
}
