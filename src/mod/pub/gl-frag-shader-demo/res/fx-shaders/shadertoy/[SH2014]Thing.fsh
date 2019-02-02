// https://www.shadertoy.com/view/4s2GWd

// iChannel0: t1
// iChannel1: t3

float kFarClip=1000.0;

vec3 vLightPos = vec3(1.5, 2.0, 3.0);			
vec3 vLightColour = vec3(1.0, 0.5, 0.05);

vec2 GetWindowCoord( const in vec2 vUV );
vec3 GetCameraRayDir( const in vec2 vWindow, const in vec3 vCameraPos, const in vec3 vCameraTarget );
vec3 GetSceneColour( in vec3 vRayOrigin,  in vec3 vRayDir );
vec3 ApplyPostFX( const in vec2 vUV, const in vec3 vInput );

vec3 vCameraOffset = vec3(0.0, 0.0, 0.0);

void main(void)
{
	vec2 vUV = gl_FragCoord.xy / iResolution.xy;

	vec2 vMouse = iMouse.xy / iResolution.xy;
	
	float fAngle = vMouse.x * 2.0 * 3.14 + 2.8;
	vec3 vCameraPos = vec3(sin(fAngle) * 10.0, vMouse.y * 10.0 - 2.0, cos(fAngle) * 10.0);

	
	vCameraOffset.z = iGlobalTime * 10.0;
	
	vec3 vCameraTarget = vec3(0.0, 0.0, 0.0);

	if(iMouse.z <= 0.0)
	{
		vCameraPos += vCameraOffset;
		vCameraTarget += vCameraOffset;

		vLightPos += vCameraOffset;
		
		vCameraPos.x += sin(iGlobalTime * 0.5);
	}

	vec3 vRayOrigin = vCameraPos;
	vec3 vRayDir = GetCameraRayDir( GetWindowCoord(vUV), vCameraPos, vCameraTarget );
	
	vec3 vResult = GetSceneColour(vRayOrigin, vRayDir);
	
	vec3 vFinal = ApplyPostFX( vUV, vResult );
	
	gl_FragColor = vec4(vFinal, 1.0);
}

// CAMERA

vec2 GetWindowCoord( const in vec2 vUV )
{
	vec2 vWindow = vUV * 2.0 - 1.0;
	vWindow.x *= iResolution.x / iResolution.y;

	return vWindow;	
}

vec3 GetCameraRayDir( const in vec2 vWindow, const in vec3 vCameraPos, const in vec3 vCameraTarget )
{
	vec3 vForward = normalize(vCameraTarget - vCameraPos);
	vec3 vRight = normalize(cross(vec3(0.0, 1.0, 0.0), vForward));
	vec3 vUp = normalize(cross(vForward, vRight));
							  
	vec3 vDir = normalize(vWindow.x * vRight + vWindow.y * vUp + vForward * 2.0);

	return vDir;
}

// POSTFX

vec3 ApplyVignetting( const in vec2 vUV, const in vec3 vInput )
{
	vec2 vOffset = (vUV - 0.5) * sqrt(2.0);
	
	float fDist = dot(vOffset, vOffset);
	
	const float kStrength = 0.5;
	
	float fShade = mix( 1.0, 1.0 - kStrength, fDist );	

	return vInput * fShade;
}

vec3 ApplyTonemap( const in vec3 vLinear )
{
	const float kExposure = 2.0;
	
	return 1.0 - exp2(vLinear * -kExposure);	
}

vec3 ApplyGamma( const in vec3 vLinear )
{
	const float kGamma = 2.2;

	return pow(vLinear, vec3(1.0/kGamma));	
}

vec3 ApplyPostFX( const in vec2 vUV, const in vec3 vInput )
{
	vec3 vTemp = ApplyVignetting( vUV, vInput );	
	
	vTemp = ApplyTonemap(vTemp);
	
	return ApplyGamma(vTemp);		
}
	
// RAYTRACE

struct C_Intersection
{
	vec3 vPos;
	float fDist;	
	vec3 vNormal;
	vec3 vUVW;
	float fObjectId;
};


float GetSceneDistance( out vec4 vOutUVW_Id, const in vec3 vPos )
{
	
	float fTileHash = (floor(vPos.x / 10.0) + floor(vPos.x / 12.0)) * 0.5;
	
	vec3 vStrandDomain = mod(vPos, 10.0) - 5.0;
		
	vStrandDomain.x += sin(vPos.y * 0.1);
	
	vStrandDomain.y += fTileHash * 10.0 + iGlobalTime * (1.0 + fract(fTileHash*123.456)) * 3.0;
	
	float fAngle = vPos.y * 0.5 + iGlobalTime* (1.0 + fract(fTileHash*123.456)) * 3.0;
	float s = sin(fAngle);
	float c = cos(fAngle);

	vec3 vStrutDomain = vStrandDomain;
	vStrutDomain.x = vStrandDomain.x * s + vStrandDomain.z * c;
	vStrutDomain.z = vStrandDomain.x * c - vStrandDomain.z * s;
	
	float fEdgeDist1 = length(vStrutDomain.xz + vec2(-1.0, 0.0)) - 0.25;
	float fEdgeDist2 = length(vStrutDomain.xz + vec2( 1.0, 0.0)) - 0.25;
	
	float fOutDist = min(fEdgeDist1, fEdgeDist2);
	vOutUVW_Id = vec4(vStrutDomain.xy, 0.0, 1.0);

	vStrutDomain.y = mod(vStrandDomain.y, 1.0) - 0.5;
	
	float fStrutDist = length(vStrutDomain.zy) - 0.1;
	
	fStrutDist = max(fStrutDist, length(vStrutDomain.xz) - 0.75);
	
	if(fStrutDist < fOutDist)
	{
		fOutDist = fStrutDist;
		vOutUVW_Id = vec4(vStrutDomain.xz, 0.0, 2.0);
	}
	//fOutDist = min(fStrutDist, fOutDist);
	
	return fOutDist;
}

vec3 GetSceneNormal(const in vec3 vPos)
{
    const float fDelta = 0.0001;

    vec3 vDir1 = vec3( 1.0, -1.0, -1.0);
    vec3 vDir2 = vec3(-1.0, -1.0,  1.0);
    vec3 vDir3 = vec3(-1.0,  1.0, -1.0);
    vec3 vDir4 = vec3( 1.0,  1.0,  1.0);
	
    vec3 vOffset1 = vDir1 * fDelta;
    vec3 vOffset2 = vDir2 * fDelta;
    vec3 vOffset3 = vDir3 * fDelta;
    vec3 vOffset4 = vDir4 * fDelta;

	vec4 vUnused;
    float f1 = GetSceneDistance( vUnused, vPos + vOffset1 );
    float f2 = GetSceneDistance( vUnused, vPos + vOffset2 );
    float f3 = GetSceneDistance( vUnused, vPos + vOffset3 );
    float f4 = GetSceneDistance( vUnused, vPos + vOffset4 );
	
    vec3 vNormal = vDir1 * f1 + vDir2 * f2 + vDir3 * f3 + vDir4 * f4;	
		
    return normalize( vNormal );
}

void TraceScene( out C_Intersection outIntersection, const in vec3 vOrigin, const in vec3 vDir )
{	
	vec4 vUVW_Id = vec4(0.0);		
	vec3 vPos = vec3(0.0);
	
	float t = 0.01;
	const int kRaymarchMaxIter = 48;
	for(int i=0; i<kRaymarchMaxIter; i++)
	{
		vPos = vOrigin + vDir * t;
		float fDist = GetSceneDistance(vUVW_Id, vPos);		
		t += fDist;
		if(abs(fDist) < 0.001)
		{
			break;
		}		
		if(t > 100.0)
		{
			t = kFarClip;
			vPos = vOrigin + vDir * t;
			vUVW_Id = vec4(0.0);
			break;
		}
	}
	
	outIntersection.fDist = t;
	outIntersection.vPos = vPos;
	outIntersection.vNormal = GetSceneNormal(vPos);
	outIntersection.vUVW = vUVW_Id.xyz;
	outIntersection.fObjectId = vUVW_Id.w;
}

#define SOFT_SHADOW

float TraceShadow( const in vec3 vOrigin, const in vec3 vDir, const in float fDist )
{
#ifndef SOFT_SHADOW
	C_Intersection shadowIntersection;
	TraceScene(shadowIntersection, vOrigin, vDir);
	if(shadowIntersection.fDist < fDist) 
	{
		return 0.0;		
	}
	
	return 1.0;
#else	
	#define kShadowIter 32
	#define kShadowFalloff 5.0
	float fShadow = 1.0;
	float t = 0.01;
	float fDelta = 1.0 / float(kShadowIter);
	for(int i=0; i<kShadowIter; i++)
	{
		vec4 vUnused;
		float d = GetSceneDistance(vUnused, vOrigin + vDir * t);
		
		fShadow = min( fShadow, kShadowFalloff * d / t );
		
		t = t + fDelta;
	}

	return clamp(fShadow, 0.0, 1.0);
#endif
}

// AMBIENT OCCLUSION

float GetAmbientOcclusion( const in vec3 vPos, const in vec3 vNormal )
{
	float fAmbientOcclusion = 0.0;
	
	float fStep = 0.1;
	float fDist = 0.0;
	for(int i=0; i<=5; i++)
	{
		fDist += fStep;
		
		vec4 vUnused;
		float fSceneDist = GetSceneDistance(vUnused, vPos + vNormal * fDist);
		
		float fAmount = (fDist - fSceneDist);
		
		fAmbientOcclusion += max(0.0, fAmount * fDist );                                  
	}
	
	return max(1.0 - fAmbientOcclusion, 0.0);
}

// LIGHTING

void AddLighting(inout vec3 vDiffuseLight, inout vec3 vSpecularLight, const in vec3 vViewDir, const in vec3 vLightDir, const in vec3 vNormal, const in float fSmoothness, const in vec3 vLightColour)
{
	float fNDotL = clamp(dot(vLightDir, vNormal), 0.0, 1.0);
	
	vDiffuseLight += vLightColour * fNDotL;
	
	vec3 vHalfAngle = normalize(-vViewDir + vLightDir);
	float fSpecularPower = exp2(4.0 + 6.0 * fSmoothness);
	float fSpecularIntensity = (fSpecularPower + 2.0) * 0.125;
	vSpecularLight += vLightColour * fSpecularIntensity * clamp(pow(dot(vHalfAngle, vNormal), fSpecularPower), 0.0, 1.0) * fNDotL;
}

void AddPointLight(inout vec3 vDiffuseLight, inout vec3 vSpecularLight, const in vec3 vViewDir, const in vec3 vPos, const in vec3 vNormal, const in float fSmoothness, const in vec3 vLightPos, const in vec3 vLightColour)
{
	vec3 vToLight = vLightPos - vPos;	
	float fDistance2 = dot(vToLight, vToLight);
	float fAttenuation = 100.0 / (fDistance2);
	vec3 vLightDir = normalize(vToLight);
	
	vec3 vShadowRayDir = vLightDir;
	vec3 vShadowRayOrigin = vPos + vShadowRayDir * 0.01;
	float fShadowFactor = TraceShadow(vShadowRayOrigin, vShadowRayDir, length(vToLight));
	
	AddLighting(vDiffuseLight, vSpecularLight, vViewDir, vLightDir, vNormal, fSmoothness, vLightColour * fShadowFactor * fAttenuation);
}

void AddPointLightFlare(inout vec3 vEmissiveGlow, const in vec3 vRayOrigin, const in vec3 vRayDir, const in float fIntersectDistance, const in vec3 vLightPos, const in vec3 vLightColour)
{
    vec3 vToLight = vLightPos - vRayOrigin;
    float fPointDot = dot(vToLight, vRayDir);
    fPointDot = clamp(fPointDot, 0.0, fIntersectDistance);

    vec3 vClosestPoint = vRayOrigin + vRayDir * fPointDot;
    float fDist = length(vClosestPoint - vLightPos);
	vEmissiveGlow += sqrt(vLightColour * 0.05 / (fDist * fDist));
}

void AddDirectionalLight(inout vec3 vDiffuseLight, inout vec3 vSpecularLight, const in vec3 vViewDir, const in vec3 vPos, const in vec3 vNormal, const in float fSmoothness, const in vec3 vLightDir, const in vec3 vLightColour)
{	
	float fAttenuation = 1.0;

	vec3 vShadowRayDir = -vLightDir;
	vec3 vShadowRayOrigin = vPos + vShadowRayDir * 0.01;
	float fShadowFactor = TraceShadow(vShadowRayOrigin, vShadowRayDir, 10.0);
	
	AddLighting(vDiffuseLight, vSpecularLight, vViewDir, -vLightDir, vNormal, fSmoothness, vLightColour * fShadowFactor * fAttenuation);	
}

void AddDirectionalLightFlareToFog(inout vec3 vFogColour, const in vec3 vRayDir, const in vec3 vLightDir, const in vec3 vLightColour)
{
	float fDirDot = clamp(dot(-vLightDir, vRayDir), 0.0, 1.0);
	float kSpreadPower = 4.0;
	vFogColour += vLightColour * pow(fDirDot, kSpreadPower);
}

// SCENE MATERIALS

void GetSurfaceInfo(out vec3 vOutAlbedo, out vec3 vOutR0, out float fOutSmoothness, out vec3 vOutBumpNormal, const in C_Intersection intersection )
{
	vOutBumpNormal = intersection.vNormal;
	
	if(intersection.fObjectId == 1.0)
	{
		vec2 vUV = intersection.vUVW.xy * 0.1;// + vec2(iGlobalTime * 0.5, 0.0);
		vOutAlbedo = texture2D(iChannel0, vUV).rgb;
		float fBumpScale = 1.0;
		
		vec2 vRes = iChannelResolution[0].xy;
		vec2 vDU = vec2(1.0, 0.0) / vRes;
		vec2 vDV = vec2(0.0, 1.0) / vRes;
		
		float fSampleW = texture2D(iChannel0, vUV - vDU).r;
		float fSampleE = texture2D(iChannel0, vUV + vDU).r;
		float fSampleN = texture2D(iChannel0, vUV - vDV).r;
		float fSampleS = texture2D(iChannel0, vUV + vDV).r;
		
		vec3 vNormalDelta = vec3(0.0);
		vNormalDelta.x += 
			( fSampleW * fSampleW
			 - fSampleE * fSampleE) * fBumpScale;
		vNormalDelta.z += 
			(fSampleN * fSampleN
			 - fSampleS * fSampleS) * fBumpScale;
		
		vOutBumpNormal = normalize(vOutBumpNormal + vNormalDelta);

		vOutAlbedo = vOutAlbedo * vOutAlbedo;	
		fOutSmoothness = vOutAlbedo.r * 0.9;//
		
		vOutR0 = vec3(0.5) * vOutAlbedo.g;
	}
	else if(intersection.fObjectId == 2.0)
	{
		vec2 vUV = intersection.vUVW.xy;
		//vec2 vUV = vec2(intersection.vUVW.z, iGlobalTime * -0.5 * 3.14 + atan(intersection.vUVW.x, intersection.vUVW.y));
		vOutAlbedo = texture2D(iChannel1, vUV).rgb;
		vOutAlbedo = vOutAlbedo * vOutAlbedo;	
		fOutSmoothness = vOutAlbedo.r;			
		vOutR0 = vec3(0.05);
	}
	
	//vOutR0 = vec3(0.9);
	//fOutSmoothness = 0.5;
}

vec3 GetSkyColour( const in vec3 vDir )
{
	vec3 vResult = vec3(0.0);
	
	//if(false)
	{
		vResult = mix(vec3(0.02, 0.04, 0.06), vec3(0.1, 0.5, 0.8), abs(vDir.y));
	}
	/*else
	{
		vec3 vEnvMap = textureCube(iChannel2, vDir).rgb;
		vEnvMap = vEnvMap * vEnvMap;
		float kEnvmapExposure = 0.5;
		vResult = -log2(1.0 - vEnvMap * kEnvmapExposure);
	}*/
	
	return vResult;	
}

float GetFogFactor(const in float fDist)
{
	float kFogDensity = 0.005;
	return exp(fDist * -kFogDensity);	
}

vec3 GetFogColour(const in vec3 vDir)
{
	return vec3(0.1, 0.25, 0.0);		
}


vec3 vSunLightColour = vec3(1.0, 0.9, 0.6) * 2.0;
vec3 vSunLightDir = normalize(vec3(0.4, -0.3, -0.5));
	
void ApplyAtmosphere(inout vec3 vColour, const in float fDist, const in vec3 vRayOrigin, const in vec3 vRayDir)
{		
	float fFogFactor = GetFogFactor(fDist);
	vec3 vFogColour = GetFogColour(vRayDir);			
	AddDirectionalLightFlareToFog(vFogColour, vRayDir, vSunLightDir, vSunLightColour);
	
	vec3 vGlow = vec3(0.0);
	AddPointLightFlare(vGlow, vRayOrigin, vRayDir, fDist, vLightPos, vLightColour);					
	
	vColour = mix(vFogColour, vColour, fFogFactor) + vGlow;	
}

// TRACING LOOP

	
vec3 GetSceneColour( in vec3 vRayOrigin,  in vec3 vRayDir )
{
	vec3 vColour = vec3(0.0);
	vec3 vRemaining = vec3(1.0);
	
	for(int i=0; i<2; i++)
	{	
		vec3 vCurrRemaining = vRemaining;
		float fShouldApply = 1.0;
		
		C_Intersection intersection;				
		TraceScene( intersection, vRayOrigin, vRayDir );

		vec3 vResult = vec3(0.0);
		vec3 vBlendFactor = vec3(0.0);
						
		if(intersection.fObjectId == 0.0)
		{
			vBlendFactor = vec3(1.0);
			fShouldApply = 0.0;
		}
		else
		{		
			vec3 vAlbedo;
			vec3 vR0;
			float fSmoothness;
			vec3 vBumpNormal;
			
			GetSurfaceInfo( vAlbedo, vR0, fSmoothness, vBumpNormal, intersection );			
		
			vec3 vDiffuseLight = vec3(0.0);
			vec3 vSpecularLight = vec3(0.0);

			AddPointLight(vDiffuseLight, vSpecularLight, vRayDir, intersection.vPos, vBumpNormal, fSmoothness, vLightPos, vLightColour);								

			AddDirectionalLight(vDiffuseLight, vSpecularLight, vRayDir, intersection.vPos, vBumpNormal, fSmoothness, vSunLightDir, vSunLightColour);								
			
			vDiffuseLight += 0.2 * GetAmbientOcclusion(intersection.vPos, vBumpNormal);

			float fSmoothFactor = fSmoothness * 0.9 + 0.1;
			vec3 vFresnel = vR0 + (1.0 - vR0) * pow(1.0 - dot(-vBumpNormal, vRayDir), 5.0) * fSmoothFactor;
			
			vResult = mix(vAlbedo * vDiffuseLight, vSpecularLight, vFresnel);		
			vBlendFactor = vFresnel;
			
			ApplyAtmosphere(vResult, intersection.fDist, vRayOrigin, vRayDir);		
			
			vRemaining *= vBlendFactor;				
			vRayDir = normalize(reflect(vRayDir, vBumpNormal));
			vRayOrigin = intersection.vPos;
		}			

		vColour += vResult * vCurrRemaining * fShouldApply;
		
		
	}

	vec3 vSkyColor = GetSkyColour(vRayDir);
	
	ApplyAtmosphere(vSkyColor, kFarClip, vRayOrigin, vRayDir);		
	
	vColour += vSkyColor * vRemaining;
	
	return vColour;
}
