// https://www.shadertoy.com/view/MdfXR4

// iChannel0: t12
// iChannel1: c0
// iChannel3: m6

// These are hydrogen atom atomic orbitals at different energy levels. They are visualized with a rotating cross-section, and of course, they move to the music. Because what would atoms be if they didn't move to music?
/*--------------------------------------------------------------------------------------
License CC0 - http://creativecommons.org/publicdomain/zero/1.0/
To the extent possible under law, the author(s) have dedicated all copyright and related and neighboring rights to this software to the public domain worldwide. This software is distributed without any warranty.
----------------------------------------------------------------------------------------
-Otavio Good

There is a bug with higher energy levels. Some day maybe I'll figure out the problem.

*/
#define MANUAL_CAMERA 0
#define MOVE_TO_MUSIC 1
#define MAX_ENERGY_PLUS_ONE 4.0

// broken
#define TRACED_SKY_SHADOWS 0

float PI=3.14159265;
vec3 sunColOrig = vec3(255.0, 208.0, 100.0)*0.5 / 255.0;
vec3 environmentSphereColor = vec3(0.3001, 0.501, 0.901)*0.5;
vec3 environmentGroundColor = vec3(0.4001, 0.25, 0.1)*0.3;

float marchingMultiplier = 1.0;
float shadow;
float localTime;
vec3 sunCol;

vec3 freq;

float distFromSphere;
vec3 normal;

float material;

vec3 saturate(vec3 a)
{
	return clamp(a, 0.0, 1.0);
}
vec2 saturate(vec2 a)
{
	return clamp(a, 0.0, 1.0);
}
float saturate(float a)
{
	return clamp(a, 0.0, 1.0);
}

float SmoothMix(float a, float b, float x)
{
	float t = x*x*(3.0 - 2.0*x);
	return mix(a, b, t);
}
vec3 RotateX(vec3 v, float rad)
{
	float cos = cos(rad);
	float sin = sin(rad);
	//if (RIGHT_HANDED_COORD)
	return vec3(v.x, cos * v.y + sin * v.z, -sin * v.y + cos * v.z);
	//else return new float3(x, cos * y - sin * z, sin * y + cos * z);
}
vec3 RotateY(vec3 v, float rad)
{
	float cos = cos(rad);
	float sin = sin(rad);
	//if (RIGHT_HANDED_COORD)
	return vec3(cos * v.x - sin * v.z, v.y, sin * v.x + cos * v.z);
	//else return new float3(cos * x + sin * z, y, -sin * x + cos * z);
}
vec3 RotateZ(vec3 v, float rad)
{
	float cos = cos(rad);
	float sin = sin(rad);
	//if (RIGHT_HANDED_COORD)
	return vec3(cos * v.x + sin * v.y, -sin * v.x + cos * v.y, v.z);
}

// k should be negative. -4.0 works nicely.
float smin(float a, float b, float k)
{
	// I'm guessing that base 2 operations are the fastest.
	return log2(exp2(k*a)+exp2(k*b))/k;
}

vec3 GetSunColorReflection(vec3 rayDir, vec3 sunDir)
{
	vec3 tex = textureCube(iChannel1, rayDir).xyz;
	return(tex*tex);
	vec3 localRay = normalize(rayDir);
	float sunIntensity = 1.0 - (dot(localRay, sunDir) * 0.5 + 0.5);
	//sunIntensity = (float)Math.Pow(sunIntensity, 14.0);
	sunIntensity = max(0.0, 0.01 / sunIntensity - 0.025);
	sunIntensity = min(sunIntensity, 40000.0);
	vec3 ground = mix(environmentGroundColor, environmentSphereColor,
					  pow(localRay.y, 0.35)*sign(localRay.y) * 0.5 + 0.5);
	return ground + sunCol * sunIntensity;
}

// http://www.chemistry.mcmaster.ca/esam/Chapter_3/section_2.html
float a0 = 0.05291772;  // Bohr radius (in nanometers maybe?)
float E = 2.71828;

float GeneralizedLaguerre(float n, float a, float x)
{
	// http://en.wikipedia.org/wiki/Laguerre_polynomials;
	//if (n == 0.0) return 1.0;
	if (n == 1.0) return (-x + a + 1.0);
	else if (n == 2.0) return ((x*x)/2.0) - (a+2.0)*x + (((a+2.0)*(a+1.0))/2.0);
	else if (n == 3.0) return (-(x*x*x)/6.0)
		+ (((a+3.0)*(x*x))/2.0)
		- (((a+2.0)*(a+3.0)*x)/2.0)
		+ (((a+1.0)*(a+2.0)*(a+3.0))/6.0);
	else if (n == 4.0)	// Not sure this n == 4 case is the right math
		return (x*x*x*x/24.0)
		- (a+4.0)*x*x*x/2.0
		+ (a+4.0)*(a+3.0)*x*x/2.0
		- (a+4.0)*(a+3.0)*(a+2.0)*x/2.0
		+ (a+4.0)*(a+3.0)*(a+2.0)*(a+1.0)/24.0;
	return 1.0;
}

float factorial(float a)
{
	if (a <= 1.0) return 1.0;
	else if (a == 2.0) return 2.0;
	else if (a == 3.0) return 6.0;
	else if (a == 4.0) return 24.0;
	else if (a == 5.0) return 120.0;
	else if (a == 6.0) return 720.0;
	else if (a == 7.0) return 5040.0;
	else if (a == 8.0) return 40320.0;
	else if (a == 9.0) return 362880.0;
	else if (a == 10.0) return 3628800.0;
	else if (a == 11.0) return 39916800.0;
	else if (a == 12.0) return 479001600.0;
	return 1.0;
}

float CalcR(float n, float l, float m, float r) {
    float Z = 1.0;  // index into periodic table (number of protons)
	float sub1 = pow((2.0 * Z) / (n * a0), 3.0);
	float sub2 = factorial(n - l - 1.0) / (2.0 * n * factorial(n + l));
	float part1 = sqrt(sub1 * sub2);
	// e ^ (-r/2*a0)
	float part2 = pow(E, (-Z * r) / (n * a0));
	// 1
	float part3 = pow((2.0 * Z * r) / (n * a0), l);
	// laguerre = -r/a0 + 2   -------- 
	float part4 = GeneralizedLaguerre(n - l - 1.0, 2.0 * l + 1.0, (2.0 * Z * r) / (n * a0));
	return part1 * part2 * part3 * part4;
 //   }
}

float SphericalHarmonicXYZ(float l, float m, vec3 pos)
{
	float x = pos.x;
	float y = pos.y;
	float z = pos.z;
	// http://en.wikipedia.org/wiki/Table_of_spherical_harmonics#Real_spherical_harmonics
/*	if (l == 0.0)
	{
		return 0.28209479;
	} else*/ if (l == 1.0)
	{
		float posm1 = pos.y;
		if (m == 0.0) posm1 = pos.z;
		else if (m == 1.0) posm1 = pos.x;
		float r = length(pos);
		return 0.488602511 * (posm1 / r);
	} else if (l == 2.0)
	{
		float r = length(pos);
		float r2 = r * r;
		if (m == -2.0)
			return 1.09254889 * ((pos.x * pos.y) / r2);
		else if (m == -1.0)
			return 1.09254889 * ((pos.y * pos.z) / r2);
		else if (m == 1.0)
			return 1.09254889 * ((pos.z * pos.x) / r2);
		else if (m == 0.0)
			return 0.31539156 * (-(pos.x * pos.x)-(pos.y * pos.y) + 2.0 * pos.z * pos.z) / r2;
		else if (m == 2.0)
			return 0.54627444 * ((pos.x * pos.x) - (pos.y * pos.y)) / r2;
	}
	else if (l == 3.0)
	{
		float r = length(pos);
		float r3 = r * r * r;
		// Math.sqrt(35/(2*Math.PI))/4
		if (m == -3.0)
			return 0.59004358 * (3.0 * x * x - y * y) * y / r3;
		// Math.sqrt(105/(Math.PI))/2
		else if (m == -2.0)
			return 2.89061144 * (x * y * z) / r3;
		// Math.sqrt(21/(2*Math.PI))/4
		else if (m == -1.0)
			return 0.45704579 * y * (4.0 * z * z - x * x - y * y) / r3;
		// Math.sqrt(7/(Math.PI))/4
		else if (m == 0.0)
			return 0.37317633 * z * (2.0 * z * z - 3.0 * x * x - 3.0 * y * y) / r3;
		// Math.sqrt(21/(2*Math.PI))/4
		else if (m == 1.0)
			return 0.45704579 * x * (4.0 * z * z - x * x - y * y) / r3;
		// Math.sqrt(105/(Math.PI))/4
		else if (m == 2.0)
			return 1.44530572 * (x * x - y * y) * z / r3;
		// Math.sqrt(35/(2*Math.PI))/4
		else if (m == 3.0)
			return 0.59004358 * (x * x - 3.0 * y * y) * x / r3;
	}
	// l = 0 is a constant.
	return 0.28209479;
}

float matMin(float a, float b, float matNum)
{
	if (a < b)
	{
		//material = 0.0;
		return a;
	}
	else
	{
		material = matNum;
		return b;
	}
}

float DistanceToObject2(vec3 p)
{
	//float f = length(p) - 1.75;
	//f += sin((p.x+iGlobalTime)*4.0)*0.05+0.05;
	//f += cos((p.y+iGlobalTime)*8.0)*0.05+0.05;

	float f = length(p);
	float currentL = 2.0;
	float currentM = mod(floor(localTime), 5.0)-2.0;
	float currentMB = mod(floor(localTime+1.0), 5.0)-2.0;
	float alpha = fract(localTime);
	float shA = (SphericalHarmonicXYZ(currentL, currentM, p));
	float shB = (SphericalHarmonicXYZ(currentL, currentMB, p));
	float sh = SmoothMix(shA, shB, alpha);
	if (sh < 0.0) material = 1.0;
	else material = 0.0;
	f = f - abs(sh);

	return f;
}

float DistanceToObject(vec3 p)
{
	//vec3 pOrigOrig = p;
	//float boxDist = -dBox(pOrigOrig, vec3(1.0,1.0,1.0)*5.0);
	//if (boxDist > 0.01) return boxDist;

	p += vec3(6.0, 3.0, 1.0);
	vec3 pOrig = p;
	vec3 c = vec3(2.0, 2.0, 2.0)*1.0;
	p = mod(p,c)-0.5*c;
	//float f = length(p) - 1.75;
	//f += sin((p.x+iGlobalTime)*4.0)*0.05+0.05;
	//f += cos((p.y+iGlobalTime)*8.0)*0.05+0.05;

	float f = length(p);
	//marchingMultiplier = clamp(0.2, 1.0, f*0.4);
	vec3 temp = floor(pOrig/c);
	float miss = 3.0;
	// table of valid numbers here: http://en.wikipedia.org/wiki/Atomic_orbital
	if (temp.x >= MAX_ENERGY_PLUS_ONE) return miss;
	if (temp.x < 1.0) return miss;//1.0
	if (temp.y < 0.0) return miss;
	if (temp.y >= temp.x) return miss;
	if (temp.z < -temp.y) return miss;
	if (temp.z >= temp.y+1.0) return miss;
	float currentN = temp.x;//floor(pOrig*c).x;
	float currentL = temp.y;//0.0;//mod(floor(localTime*0.25), 4.0);
	float currentM = temp.z;// mod(floor(localTime), 5.0)-2.0;
	//float currentMB = mod(floor(localTime+1.0), 5.0)-2.0;
	float alpha = fract(localTime);

	float shA = (SphericalHarmonicXYZ(currentL, currentM, p));
	float coolR = CalcR(currentN, currentL, currentM, length(p));
	float waveFunc = (shA * coolR);
	shA = waveFunc;

/*	float shB = (SphericalHarmonicXYZ(currentL, currentMB, p));
	coolR = CalcR(currentN, currentL, currentMB, length(p));
	waveFunc = (shB * coolR);
	shB = waveFunc;
*/
	float sh = shA;// SmoothMix(shA, shB, alpha);

	if (sh < 0.0) material = 1.0;
	else material = 0.0;
	f = f - abs(sh);
	// do the cross-section slice
	f = max(f, p.y*sin(localTime)+p.z*cos(localTime));
	// move to the music
#if MOVE_TO_MUSIC
	f += sin((p.x+localTime)*8.0)*0.2*saturate(freq.x-0.5);
	float hf = sin((p.y+localTime)*32.0)*0.15*saturate(freq.y-0.3);
	f -= hf;
#endif

	return f;
}

vec4 tex3d(vec3 pos, vec3 normal)
{
	// loook up brick texture, blended across xyz axis based on normal.
	vec4 texX = texture2D(iChannel0, pos.yz);
	vec4 texY = texture2D(iChannel0, pos.xz);
	vec4 texZ = texture2D(iChannel0, pos.xy);
	vec4 tex = mix(texX, texZ, abs(normal.z));
	tex = mix(tex, texY, abs(normal.y));//.zxyw;
	return tex;
}

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------
void main(void)
{
	localTime = iGlobalTime;
	vec2 uv = gl_FragCoord.xy/iResolution.xy * 2.0 - 1.0;// - 0.5;

	freq.x = texture2D(iChannel3, vec2(0.0, 0.0)).x;
	freq.y = texture2D(iChannel3, vec2(0.1, 0.0)).x;
	freq.z = texture2D(iChannel3, vec2(0.3, 0.0)).x;

	// Camera up vector.
	vec3 camUp=vec3(0,1,0); // vuv

	// Camera lookat.
#if MANUAL_CAMERA
	vec3 camLookat=vec3(0,0.0,0);	// vrp
	vec3 camPos=vec3(1.0, 1.0, 1.0)*3.99;

	float mx=iMouse.x/iResolution.x*PI*2.0;// + iGlobalTime * 0.2;
	float my=-iMouse.y/iResolution.y*10.0;// + sin(iGlobalTime * 0.3)*0.2+0.2;//*PI/2.01;
	camPos=vec3(cos(my)*cos(mx),sin(my),cos(my)*sin(mx))*camPos; 	// prp
#else
	float camSpeed = 0.25;
	vec3 camLookat=vec3(0,0.0,0);	// vrp
	vec3 camPos=vec3(0.0, 0.0, 0.0);
	float camRad = 3.25;
	camPos = vec3(sin(localTime*camSpeed)*camRad, 0.0, cos(localTime*camSpeed)*camRad);

#endif

	// Camera setup.
	vec3 camVec=normalize(camLookat - camPos);//vpn
	vec3 sideNorm=normalize(cross(camUp, camVec));	// u
	vec3 upNorm=cross(camVec, sideNorm);//v
	vec3 worldFacing=(camPos + camVec);//vcv
	vec3 worldPix = worldFacing + uv.x * sideNorm * (iResolution.x/iResolution.y) + uv.y * upNorm;//scrCoord
	vec3 relVec = normalize(worldPix - camPos);//scp

	vec3 sunDir = normalize(vec3(-0.95, -3.0, -0.95));
	sunCol = sunColOrig;

	// ------------------------------ Ray march -----------------------------------
	float dist = 0.02;
	float t = 0.1;
	float maxDepth = 16.0;
	vec3 pos = vec3(0,0,0);
	vec3 lastPos = vec3(1000000.0, 0.0, 0.0);
	// ray marching time
	float marchCount = 0.0;
	for (int i = 0; i < 200; i++)
	{
		if ((t > maxDepth) || (abs(dist) < 0.001))
		{
			break;	// break doesn't work on some machines
			continue;	// so do a continue for those
		}
		pos = camPos + relVec * t;
		dist = DistanceToObject(pos);
		t += dist * 0.2;//marchingMultiplier;//*0.2;	// because deformations mess up distance function.
		marchCount += 1.0;
	}
	float finalMaterial = material;
	marchingMultiplier = 1.0;

	// ----------------------------------------------------------------------------


	vec3 finalColor = GetSunColorReflection(relVec, -sunDir);// + vec3(0.1, 0.1, 0.1);

	// calculate normal from distance field
	vec3 smallVec = vec3(0.005, 0, 0);
	vec3 dx = pos - smallVec.xyy;
	vec3 dy = pos - smallVec.yxy;
	vec3 dz = pos - smallVec.yyx;
	vec3 normal = vec3(dist - DistanceToObject(dx),
					   dist - DistanceToObject(dy),
					   dist - DistanceToObject(dz));
	normal = normalize(normal);

	// -------------------------- Shadow trace --------------------------
	shadow = 1000000.0;
	//for (int i = 1; i < 8; i++)
	//{
	//	vec3 midPos = mix(pos, oculus, float(i) / 64.0);
	//	float shadow0 = max(0.0, DistanceToObject(midPos));
	//	shadow = min(shadow, shadow0);
	//}
/*	float dist2 = 0.02;
	float t2 = 0.1;
	float maxDepth2 = 40.0;
	vec3 pos2 = vec3(0,0,0);
	// ray marching time
	float mCount = 0.0;
	for (int i = 0; i < 32; i++)
	{
		if ((t2 > maxDepth2) || (abs(dist2) < 0.001)) continue;	// break DOESN'T WORK!!! ARRRGGG!
		pos2 = (pos + normal*0.02) + normalize(oculus - pos) * t2;
		dist2 = DistanceToObject(pos2);
		t2 += dist2 * marchingMultplier;	// because deformations mess up distance function.
		mCount++;
	}
	if (dist2 < 0.01) shadow = 0.0;*/
	shadow = saturate(shadow * 300.9);

//	float shadow0 = max(0.0, DistanceToObject(mix(oculus, pos, 0.333)));
//	float shadow1 = max(0.0, DistanceToObject(mix(oculus, pos, 0.666)));
//	shadow0 = min(shadow0, shadow1);
	// ----------------------------------------------------------------------------

	// calculate ambient occlusion with 3 extra distance field queries
	float ambient = DistanceToObject(pos + normal * 0.5);
	ambient += DistanceToObject(pos + normal * 0.25);
	ambient += DistanceToObject(pos + normal * 0.125);
	ambient = max(0.1, ambient);	// tone down ambient with a pow and min clamp it.
	ambient = saturate(ambient);

	vec3 tempPos = pos;
	// look up brick texture, blended across xyz axis based on normal.
	vec4 tex = tex3d(tempPos, normal);
	tex = tex * tex;	// gamma correct

	// materials
	vec3 ref = reflect(relVec, normal);

	//tex.xyz = vec3(1.0,1.0,1.0)*0.8;
	if (finalMaterial == 1.0)
	{
		tex.xyz *= vec3(1.0, 0.175, 0.175)*1.0;
	}
	else
	{
		tex.xyz *= vec3(0.5, 1.0, 0.5)*1.0;
	}
	tex.xyz *= 1.6;	// texture seems a bit dark, maybe not normalized
	//tex.xyz = vec3(1.0,1.0,1.0);

	// if ray marching found an intersection, then calculate lighting and stuff
	if (t <= maxDepth)
	{
		vec3 lightDir = sunDir;
		float lenLightDir = length(lightDir);
		float skyMult = max(0.0, -lightDir.y) * max(0.0, dot(-lightDir, normal));
		skyMult *= 0.55 / (lenLightDir*lenLightDir);
		//vec3 envLight = environmentSphereColor;// * skyMult;// + environmentGroundColor * (normal.y * 0.5 + 0.5);
		vec3 envLight = mix(environmentGroundColor, environmentSphereColor, (normal.y * 0.5 + 0.5));
		envLight *= shadow;// * skyMult;
		//envLight += environmentSphereColor * 0.045;	// ambient is lame, but better than black.
		//vec3 domeLight = vec3(1.0, 0.1, 0.5) * (normal.y * 0.5 + 0.5) * min(ambient, shadow) * 0.15;

		vec3 sunDirect = max(0.0, dot(-sunDir, normal)) * sunCol * 3.0;
		vec3 sunIndirect;// = 0.25*pow(saturate((3.5-distance(flatPos, flatOculus))), 2.0) * sunCol * 0.31;
		finalColor = (envLight) * tex.xyz;
		finalColor *= vec3(1.0,1.0,1.0) * ambient;
		finalColor += (sunDirect) * tex.xyz;// * ambient;
		vec3 texBack = textureCube(iChannel1, ref).xyz;
		vec3 texDark = pow(texBack, vec3(1.0,1.0,1.0)*50.0).zzz;	// fake hdr texture
		texBack += texDark*0.5;
		float nooks = pow(saturate(tex.x + 0.85), 4.0);
		texBack *= nooks;
		finalColor = mix(finalColor, texBack*texBack*texBack*ambient * 0.995, 0.3);
		//finalColor += texBack*texBack*texBack*ambient * 0.5;
		finalColor = mix(finalColor, vec3(0.015,0.015,0.015), pow(saturate(distance(pos, camPos)*0.075), 0.7) );
		//finalColor = vec3(1.0,1.0,1.0)*pow(tex.x + 0.85, 14.0);
	}
	//finalColor += vec3(0.9,0.3,1.0)* marchCount/256.0;
	//gl_FragColor = vec4(clamp(finalColor, 0.0, 1.0),1.0);
	gl_FragColor = vec4(sqrt(clamp(finalColor*1.0, 0.0, 1.0)),1.0);
}
