// https://www.shadertoy.com/view/ldlSz4

// iChannel0: t1

// a VERY simple raytracer to run in a shader You can change the depth at line 203 to decrease/increase secondary rays
//simple shape struct that supports creation of either planes or spheres
//hence the variable names - pos for spheres, normals for planes etc
struct Shape
{
	vec3  posOrNormal;
	vec4  col;
	float radiusOrDistance;
	bool  isPlane;
};
	
int numShapes = 6;
	
//rotation matrices
mat3 RotationMatrixY = mat3( cos( iGlobalTime ),  0.0,   sin( iGlobalTime ), 
			    				0.0,              0.0,   0.0,
			             	-sin( iGlobalTime ),  0.0,   cos(iGlobalTime) );

mat3 RotationMatrixX = mat3( 1.0,        0.0,            0.0,
			    			0.0,  cos( iGlobalTime ),    -sin(iGlobalTime), 
			             	0.0,  sin(iGlobalTime),      cos(iGlobalTime) );

//camera origin	
vec3 camOrigin = vec3(0.0,2.0,-3.0);
vec3 lookAt    = vec3(0.0,0.0,0.0);

//one light
vec3 lightPos[2];


float fov = 30.0 * 3.1415926/180.0;

float dist = 0.5/tan(fov * 0.5);

const float NO_INTERSECTION = 1000.0;

//get the texture co-ordinates for the sphere at the
//ray interesection point - because these aren't rasterised spheres 
//we are unable to make the spheres "rotate" around their own axis , we have to 
//rotate the vectors used for the sampling
vec2 GetUVForSphere(vec3 centerToIntersection)
{
	vec3 center = RotationMatrixX * centerToIntersection;
	vec3 north = RotationMatrixX * vec3(0.0,1.0,0.0); 
	vec3 equator = RotationMatrixX * vec3(1.0,0.0,0.0);
	float numerator = acos(dot(-vec3(0.0,1.0,0.0),center));
	float v = numerator/3.141259;
	float u;
	
	float possU = (acos(dot( equator,center))/sin(numerator))/(2.0*3.141259);
	
	if(dot(cross(vec3(0.0,1.0,0.0), equator),center) > 0.0)
	{
		u = possU;
	}
	else
	{
		u = 1.0 - possU;
	}
		
	return vec2(u,v);
}
	
float IntersectPlane(vec3 dir,vec3 origin,Shape inShape)
{
  float val1 = -(dot(origin,inShape.posOrNormal)+ inShape.radiusOrDistance);
  float val2 = dot(dir,inShape.posOrNormal);
  float tVal = val1/val2;

  return tVal > 0.0 ? tVal : NO_INTERSECTION;	
	//return NO_INTERSECTION;
}

float IntersectSphere(vec3 dir,vec3 origin,Shape inShape)
{
	//calc all the dot products necessary here
	float radiusSquared = inShape.radiusOrDistance * inShape.radiusOrDistance;
	float rayOrigDot    = dot(origin,origin);
	float rayOriDirDot  = dot(dir,origin);
	float rayDirDot     = dot(dir,dir);
	float spherePosDot = dot(inShape.posOrNormal,inShape.posOrNormal);
	
	//here are the variables for the quadratic equation
	//a is the dot prod between the ray direction and itself
	float b = 2.0 * (rayOriDirDot - dot(dir,inShape.posOrNormal));
	float c = rayOrigDot + spherePosDot - 2.0 * dot(origin,inShape.posOrNormal) - radiusSquared;

	
	//the value for a is raydirdot
	float discriminant = (b*b) - 4.0 * rayDirDot * c;

	if(discriminant < 0.0) return NO_INTERSECTION;

	discriminant = sqrt(discriminant);

	if(-b+discriminant < 0.0) return NO_INTERSECTION;

	float denom = 1.0/(2.0 * rayDirDot);
	float t1 = (-b + discriminant) * denom;
	float t2 = (-b - discriminant) * denom;

	return t1 < t2 ? t1 : t2; 
}


vec3 CreateRay()
{
	vec3 dir      = normalize(lookAt - camOrigin); 
	vec3 upVec    = normalize(cross(vec3(1.0,0.0,0.0),dir));	
	vec3 rightVec = normalize(cross(dir,upVec));
	vec3 factor1  = (0.5 - (gl_FragCoord.y/iResolution.y)) * upVec;
	vec3 factor2  = ((gl_FragCoord.x/iResolution.x)-0.5) * rightVec;
	dir = dir * dist + factor1 + factor2;  
	return normalize(dir);
}

//once we know the instersect/contact point for the ray lets find the colour
vec4 GenColor(vec3 contactPoint,Shape inShape[6],int index,vec3 dir,vec3 normal)
{
	vec4 col = vec4(0.0,0.0,0.0,1.0);
	
	for(int light = 0; light < 2; light++)
	{
		for(int s = 0; s < 6; s++)
		{
			//because of the way glsl is compiled we can't use
			//inshape[index] in the code, so we have to iterate through each shape and 
			//use a test
			if(s == index)
			{
				vec3 lightDir = normalize(lightPos[light]-contactPoint);
				vec3 viewVec  =  normalize(contactPoint - camOrigin);
				vec3 reflectVec = reflect(-lightDir,normal);
				vec4 diff = max(dot(normal,lightDir),0.0) * inShape[s].col;
				vec4 spec = pow(max(dot(viewVec,reflectVec),0.0),16.0) * vec4(1.0,1.0,1.0,1.0);
				
				if(!inShape[s].isPlane)
				{
					vec2 coords = GetUVForSphere(normalize(inShape[s].posOrNormal-contactPoint));
					diff *= texture2D(iChannel0,coords);
				}
				
				col += clamp(diff + spec,0.0,1.0);
			}
		}
	}
	
	return clamp(col,0.0,1.0);
}

//create the shapes for this frame, give new positions for the spheres
//using rotation matrices and the current value of the iGlobalTime
//to create an "orbit"
void CreateShapes(out Shape shapes[6])
{
	shapes[0].posOrNormal       = RotationMatrixY * vec3(0.0,0.0,0.0);
	shapes[0].radiusOrDistance  = 0.1;
	shapes[0].col               = vec4(1.0,0.0,0.0,1.0);
	shapes[0].isPlane           = false;
	
	shapes[1].posOrNormal       = RotationMatrixY * vec3(0.4,0.0,-0.2);
	shapes[1].radiusOrDistance  = 0.15;
	shapes[1].col               = vec4(0.0,1.0,0.0,1.0);
	shapes[1].isPlane           = false;
	
	shapes[2].posOrNormal       = RotationMatrixY * vec3(-0.2,0.0,0.2);
	shapes[2].radiusOrDistance  = 0.05;
	shapes[2].col               = vec4(0.5,0.0,1.0,1.0);
	shapes[2].isPlane           = false;
	
	shapes[3].posOrNormal       = RotationMatrixY * vec3(0.1,0.0,-0.5);
	shapes[3].radiusOrDistance  = 0.05;
	shapes[3].col               = vec4(0.6,0.2,0.0,1.0);
	shapes[3].isPlane           = false;
	
	shapes[4].posOrNormal       = RotationMatrixY * vec3(0.6,0.0,0.5);
	shapes[4].radiusOrDistance  = 0.1;
	shapes[4].col               = vec4(1.0,0.5,0.5,1.0);
	shapes[4].isPlane           = false;
	
	shapes[5].posOrNormal		= vec3(0.0,1.0,0.0);
	shapes[5].radiusOrDistance  = 0.5;
	shapes[5].col               = vec4(0.5,0.2,0.5,1.0);
	shapes[5].isPlane			= true;
	
	lightPos[0] = vec3(0.0,3.0,0.0);
	lightPos[1] = vec3(8.0,0.0,-5.0);
}
	
void main(void)
{
	Shape shapes[6];
	CreateShapes(shapes);
	
	vec3 ray = CreateRay();
	vec4 col = vec4(0.0,0.0,0.0,1.0);
	vec3 origin = camOrigin;
	vec3 contactPoint = vec3(0.0,0.0,0.0);
	vec3 normal = vec3(0.0,0.0,0.0);
	float t = NO_INTERSECTION;
	int index = -1;
	
	
	for(int depth = 0; depth < 2; depth++)
	{
		float refCoeff = 1.0;
		
		float newT;
		for(int s = 0; s < 6; s++)
		{
			
			
		  if(shapes[s].isPlane)
		  {
			  newT = IntersectPlane(ray,origin,shapes[s]);
		  }
		  else
		  {
			  newT = IntersectSphere(ray,origin,shapes[s]);
		  }
			  
		  if(newT < t && newT > 0.0)
		  {
			  t = newT;
			  index = s;
			  contactPoint = origin + t * ray;
			  
			  if(shapes[s].isPlane)
			  {  
			    normal = shapes[s].posOrNormal;
			  }
			  else
			  {
				  
				normal   = normalize(contactPoint - shapes[s].posOrNormal);
			  }
			  
		  }
		}
		
		if(index != -1)
		{
			col += refCoeff * GenColor(contactPoint,shapes,index,ray,normal);
			ray = normalize(reflect(ray,normal));
			origin = contactPoint + ray * 0.1;
			refCoeff * 0.5;
			t = NO_INTERSECTION;
		}
		else
		{
			break;
		}
		
		
	}
	
	gl_FragColor = clamp(col,0.0,1.0);
}
