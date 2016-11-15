// https://www.shadertoy.com/view/Mdl3z8

// iChannel2: t12

// RedSpace
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
// Thanks to iq, P_Malin and PauloFalcao for all the excellent code examples


#define d_StartDist 0.0
#define d_FarClip 400.0
#define d_maxIterations 50

#define dEpsilon 0.01
// structures
struct Ray 
{
	vec3 origin;
	vec3 direction;
	float farPlane;
};

struct Surface
{
	vec3 normal;
};

struct Shading
{
	vec3 diffuse;
	vec3 specular;	
};

struct Directional_Light
{
	vec3 dir;
	vec3 color;
	float intensity;
};
	
struct HitInfo
{
	vec3 pos;
	float dist;
	float id;
};
	
// scene description
vec3 background()
{
	float hort = 1.0 - clamp((gl_FragCoord.x + 0.2)/iResolution.x, 0.1, 1.0);
	float vert = 1.0 - clamp((gl_FragCoord.y + 0.2)/(iResolution.y), 0.1, 1.0);
	return vec3(0.8*(hort/2.0 + vert/2.0));
}

vec2 distUnion(const in vec2 obj1, const in vec2 obj2)
{
	return mix(obj1, obj2, step(obj2.x, obj1.x)); 
}

vec3 rotate(in vec3 pos, in float anglex, in float angley, in float anglez)
{
	mat3 rotM;
	rotM[0].x = cos(angley) * cos(anglez);
	rotM[0].y = -cos(angley) * sin(anglez);
	rotM[0].z = sin(angley);
	rotM[1].x = cos(anglex)*sin(anglez) + sin(anglex)*sin(angley)*cos(anglez);
	rotM[1].y = cos(anglex)*cos(anglez) - sin(anglex)*sin(angley)*sin(anglez);
	rotM[1].z = -sin(anglex)*cos(angley);
	rotM[2].x = sin(anglex)*sin(anglez) - cos(anglex)*sin(angley)*cos(anglez);
	rotM[2].y = sin(anglex)*cos(anglez) + cos(anglex)*sin(angley)*sin(anglez);
	rotM[2].z = cos(anglex)*cos(angley);
	
	return rotM * pos;
}

vec3 rotsim2(vec3 p, float s)
{
	vec3 result = p;
	float r1 = -3.1416/(s*(4.5));
	result = rotate(p, 0.0, r1, 0.0);
	float r2 = floor(atan(result.x, result.z)/3.1416*s)*(3.1416/s);
	result = rotate(p, 0.0, r2, 0.0);
	//result = rotate(p, 0.0, 3.1416, 0.0);
	return result;
}

vec2 sceneDescription(const in vec3 pos)
{
	vec2 result = vec2(1000.0, -1.0);
	
	// sphere
	vec3 translate = vec3(0.0, 0.0, 55.0);
	vec2 distSphere = vec2(length(pos + translate) - 2.0, 1.0); 
	result = distUnion(result, distSphere);
	
	//box
	vec3 pos2 = pos+translate;
	pos2 = rotate (pos2, -0.2, iGlobalTime*0.4, -0.2);
	pos2 = rotsim2(pos2,7.0);
	vec3 orbit = rotate(pos2, 0.0, 3.1416, 0.0);
	vec3 translate2 = vec3(-10.0, -1.0, 16.0);
	vec3 ot = orbit+translate2;
	ot = vec3(mod(ot.x, 8.0) - 4.0, ot.yz);
	vec3 rpos = rotate(ot, iGlobalTime, -iGlobalTime, iGlobalTime*2.0);
	vec2 distBox = vec2(length(max(abs(rpos) - 0.8, 0.0)), 2.0);
	
	result = distUnion(result, distBox);
	
	return result;
}

Directional_Light getDLight(const in int index)
{
	Directional_Light dl;
	if(index == 0){
		dl.dir = normalize(vec3(0.8, 0.3, -1.0));
		dl.color = vec3(1.0, 1.0, 1.0);
		dl.intensity = 1.0;
	}
	else if(index == 1) {
		dl.dir = normalize(vec3(-0.5, -0.5, 0.5));
		dl.color = vec3(1.0, 1.0, 1.0);
		dl.intensity = 1.0;
	}
	return dl;
}

// raymarcher
vec3 pointNormal(const in vec3 pos) 
{
	const float delta = 0.025;
	vec3 offs1 = vec3(  delta, -delta, -delta );
	vec3 offs2 = vec3( -delta, -delta,  delta );
	vec3 offs3 = vec3( -delta,  delta, -delta );
	vec3 offs4 = vec3(  delta,  delta,  delta );
	
	float f1 = sceneDescription(pos + offs1).x;
	float f2 = sceneDescription(pos + offs2).x;
	float f3 = sceneDescription(pos + offs3).x;
	float f4 = sceneDescription(pos + offs4).x;
	
	vec3 normal = offs1 * f1 + offs2 * f2 + offs3 * f3 + offs4 * f4;
	
	return normalize(normal);
}

Ray ProjectRay( in vec3 lookfrom, in vec3 lookat, in float zf, in vec2 dof )
{
	Ray newRay;
	vec2 uv = gl_FragCoord.xy / iResolution.xy;
	vec2 view = uv * 2.0 - 1.0;
	
	float aspectRatio = iResolution.x / iResolution.y;
	
	view.y /= aspectRatio;
	vec3 up = vec3(0.0, 1.0, 0.0);
	
	newRay.origin = lookfrom;
	vec3 n = normalize(lookat - lookfrom);
	vec3 u = normalize(cross(up, n));
	vec3 v = normalize(cross(u,n));
	
	Ray dofRay;
	float Noise = (texture2D(iChannel2, vec2(uv.x+iGlobalTime*3.14, uv.y)).r + texture2D(iChannel2, vec2(uv.x, uv.y+iGlobalTime*-1.37)).r);
	dofRay.origin = lookfrom + vec3(Noise*dof*0.5, 0.0);
	
	newRay.direction = normalize(zf * n + view.x * u + view.y * v);
	
	dofRay.direction = normalize(newRay.direction*40.0 - dofRay.origin);
	dofRay.farPlane = d_FarClip;
	return dofRay;
}


HitInfo intersect( const in Ray r ) 
{	
	HitInfo result = HitInfo(r.origin, 0.0, 0.0);
	
	for(int i = 0; i < d_maxIterations; i++)
	{
		result.pos = r.origin + r.direction * result.dist;
		vec2 sceneDist = sceneDescription(result.pos);
		if(sceneDist.x <= dEpsilon)
		{
			break;
		}
		result.dist = result.dist + sceneDist.x;
		result.id = sceneDist.y;
	}
	if(result.dist >= r.farPlane)
	{
		result.pos = r.origin;
		result.dist = r.farPlane;
		result.id = 0.0;
	}
	
	return result;
}
// light and shade

Shading applyDLight(in Directional_Light dL, in vec3 pos, in vec3 inDir, in vec3 norm)
{
	Shading shade;
	vec3 lightDir = -dL.dir;
	
	vec3 inL = dL.color * max(0.0, dot(lightDir, norm));
	shade.diffuse =  inL * dL.intensity;
	
	vec3 halfAngle = normalize(lightDir - inDir);
	float NH = max(0.0, dot(halfAngle, norm));
	
	
	shade.specular = (pow(NH, 300.0) * inL) * dL.intensity;
	return shade;
}

vec3 shading(in vec3 pos, in vec3 inDir, in float objectID)
{
	
	const int numDLights = 1;
	
	vec3 n = pointNormal(pos);
	
	Shading shade;
	shade.diffuse = vec3(0.0);
	shade.specular = vec3(0.0);
	
	shade.diffuse += vec3(0.1);
	
	
	Directional_Light dL = getDLight(0);
	Shading dLcontribution = applyDLight(dL, pos, inDir, n);
	shade.diffuse += abs(dLcontribution.diffuse);
	shade.specular += abs(dLcontribution.specular);
	
	dL = getDLight(1);
	dLcontribution = applyDLight(dL, pos, inDir, n);
	shade.diffuse += abs(dLcontribution.diffuse);

	
	//material
	shade.diffuse *= vec3(0.9, 0.1, 0.05);
	shade.specular *= vec3(1.0);
	
	return vec3(shade.diffuse + shade.specular);
}

void main( void )
{	
	vec3 eye = vec3(0.0,0.0,5.0);
	vec3 lookat = vec3(0.0, 0.0, 0.0);
	
	vec2 dof[4];
	dof[0] = vec2(0.5*texture2D(iChannel2, vec2(0.0, -iGlobalTime*1.61)).s, 
				  -0.42*texture2D(iChannel2, vec2(iGlobalTime*1.07, 0.7)).t);
	dof[1] = vec2(0.1*texture2D(iChannel2, vec2(1.4, iGlobalTime*0.3)).s, 
				  0.92*texture2D(iChannel2, vec2(-iGlobalTime*1.44, 0.3)).t);
	dof[2] = vec2(-0.58*texture2D(iChannel2, vec2(0.3, iGlobalTime*3.0)).s, 
				  -0.81*texture2D(iChannel2, vec2(-iGlobalTime*4.2, 0.5)).t);
	dof[3] = vec2(0.95*texture2D(iChannel2, vec2(0.6, iGlobalTime*2.71)).s, 
				  0.52*texture2D(iChannel2, vec2(iGlobalTime*0.88, 0.9)).t);
	
	vec3 totalColor = vec3(0.0);
	vec3 rayContrib[4]; 
	
	for(int i = 0; i < 3; i++)
	{
		Ray fragRay = ProjectRay(eye, lookat, 3.0, dof[i]);
		HitInfo obj = intersect(fragRay);
		vec3 color = background( );
		if(obj.id > 0.5) {
			color = shading(obj.pos, fragRay.direction, obj.id);	
		}
		rayContrib[i] = color;
	}
	
	totalColor = (rayContrib[0] + rayContrib[1] + rayContrib[2])/ 3.0;
	
	gl_FragColor = vec4( totalColor, 1.0 );
}
