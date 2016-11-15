// https://www.shadertoy.com/view/ld2GWc
// Before I apply cross-product anywhere, I always visualize it in my head... however, sometimes I confuse when its UP or when its DOWN.. so I've made this little shader to show it's behavior. Axis: [Left -X+ Right] [Down -Y+ Up] [Back -Z+ Forward]
#define PI					3.14159265359
#define PIH 				(PI/2.0)
#define PIQ 				(PI/4.0)
#define PI2 				(PI*2.0)
#define MARCH_SCALE			0.95
#define MARCH_STEPS			48
#define PRECISION			0.001

#define LIGHT_POS			vec3(0.0, 0.0, -5.0)
#define COLOR_1				vec3(-2.5, 2.5, 4.0)
#define COLOR_2				vec3(4.0, 1.5, -1.0)

// Used inside the scene function, requires you to declare float depthCurr = 1000.0; float depthPrev = 1000.0; and float matID = -1.0;
#define OBJ(inOBJ, inMatID) depthPrev=depthCurr; depthCurr=min(depthCurr, inOBJ); if(depthCurr < depthPrev) matID = inMatID;

vec3 GetRayDir(vec2 inTC, vec2 inAspectRatio)	{return normalize(vec3((-0.5 + inTC)*1.2 * inAspectRatio, 1.0));}
mat3 RotX(float a)								{float s = sin(a); float c = cos(a); return mat3(1,0,0,0,c,s,0,-s,c);}
mat3 RotY(float a)								{float s = sin(a); float c = cos(a); return mat3(c,0,-s,0,1,0,s,0,c);}
mat3 RotZ(float a)								{float s = sin(a); float c = cos(a); return mat3(c,s,0,-s,c,0,0,0,1);}

// Global variables
vec2 CrossObj(vec3 p)
{
	float depthCurr	= 1000.0;
	float depthPrev	= 1000.0;
	float matID		= -1.0;
	
	const float cylinderHeight	= 1.0;
	const float cylinderRadius	= 0.005;
	
	
	vec3 crossp					= cross(vec3(1, 0, 0), vec3(sin(iGlobalTime),0,cos(iGlobalTime)));
	float crossHeight			= crossp.y;
	
	OBJ(max(length(p.zy)-cylinderRadius, abs(p.x-cylinderHeight*0.5)-cylinderHeight*0.5), 0.0); // X - Axis
	
	OBJ(max(length(p.xz)-cylinderRadius, abs(p.y-crossHeight*0.5)-abs(crossHeight)*0.5), 1.0);	// Y - Axis
	
	// We rotate only the Z axis vector
	p = RotY(iGlobalTime) * p;
	OBJ(max(length(p.xy)-cylinderRadius, abs(p.z-cylinderHeight*0.5)-cylinderHeight*0.5), 2.0); // Z - Axis
	
	OBJ(length(p)-0.05, 3.0); // Z - Axis
	
	return vec2(depthCurr, matID);
}


vec3 GetColor(float id)
{
	// Background color
	vec3 color = vec3(0,0,0);
	
	// Material Colors
	if(id == 0.0)color=vec3(1.0, 0.0, 0.0); // Red
	if(id == 1.0)color=vec3(0.0, 1.0, 0.0); // Green
	if(id == 2.0)color=vec3(0.0, 0.0, 1.0); // Blue
	if(id == 3.0)color=vec3(0.4, 0.4, 0.4); // Grey
	
	return color;
}


vec2 Intersect(vec3 ro, vec3 rd)
{		
	vec2 r		= vec2(1000.0, -1.0);
	float z		= 1.1;
	float matID	= -1.0;
	
	for(int i=0; i<MARCH_STEPS; i++)
	{
		r = CrossObj(ro + rd*z);
		if(r.x < PRECISION)
			continue;
		z		+=r.x*MARCH_SCALE;
		matID	= r.y;
	}
	
	if(z > 3.1)
		z = 0.0;
	
	return vec2(z, matID);
}



void main(void)
{
	vec2 uv		= gl_FragCoord.xy / iResolution.xy;
	
	// Construct simple ray
	mat3 xrot	= RotX(0.5);
	vec2 aspect	= vec2(iResolution.x/iResolution.y, 1.0);
	vec3 ro		= xrot * vec3(0, 0.0, -2);
	vec3 rd		= xrot * GetRayDir(uv, aspect);
	
	// Draw background color
	vec3 c1		= mix(COLOR_2, COLOR_1, pow(uv.x, 2.0));
	vec3 c2		= mix(COLOR_2, COLOR_1, pow(uv.y, 2.0));
	vec3 color	= mix(c1, c2, 0.5) * 0.1;

	vec2 res	= Intersect(ro, rd);
	vec3 p;
	vec3 n;
	if(res.x > 0.0)
		color = GetColor(res.y);

	gl_FragColor = vec4(color, 1.0);
}
