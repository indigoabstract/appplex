// https://www.shadertoy.com/view/lsjXWh

// iChannel0: t6

//-------------------------------------------------------------
// Thanks to Beautypi (Iq and Pol Jeremias) for this wonderful sharing environment
//-------------------------------------------------------------
// Raymarching techniques by Iq
//		http://www.iquilezles.org/www/articles
//		Thanks a lot for all your papers and the clear explanations :)
//-------------------------------------------------------------
// Distance functions
//		http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
//		https://www.shadertoy.com/view/Xds3zN
//-------------------------------------------------------------
// CookTorrance shader:
// 		- Background physics and math of shading (SIGGRAPH2013 papers)
//      - Physically Based Shading at Pixar (SIGGRAPH2013 papers)
//		- Fresnel value set to 1.0 for all materials, I've played with
//		  roughness and reflectivity to achieve the needed result
//-------------------------------------------------------------
// Music only for atmosphere:
//		Most geometric person by Noby
//-------------------------------------------------------------
// Photo inspiration: 
//		http://instagram.com/p/tQ9EaizRIC by Roland Levesque
//-------------------------------------------------------------

//STRUCTS -----------------------------------------------------
//Materials----------------------------------------------------
struct material 
{
	vec3		albedo;				// Albedo color
	float		roughness;			// Roughness
	float		reflectivity;		// Fraction of diffuse reflection
};

//RayIntersect-------------------------------------------------
struct rayIntersect 
{
	vec3		mPos;				//Pos
	vec3		nor; 				//Normal
	float		dist;				//Distance
	material	mat; 				//Object material
};
//-------------------------------------------------------------

//Lights ------------------------------------------------------
struct lightTube
{
	vec3 tubeStart;
    vec3 tubeEnd;
	vec3 color;
};
//-------------------------------------------------------------
//END STRUCTS -------------------------------------------------

//DEFINE VALUES -----------------------------------------------
#define PI 3.14159

#define EPSILON		0.000001	//min dist distance
#define FARCLIP 	35.0		//Far clip distance

#define AMBIENT_COLOR vec3(1.0, 0.94510, 0.89412)

#define BOXROOM_SIZE vec3(4.1, 2.6, 14.5)
#define BOXROOM_POS vec3(0.0, 2.9, -9.5)

#define ELEVATOR_SIZE vec3(0.3, 2.15, 1.0)
#define ELEVATOR_POS vec3(1.4, -2.85, 2.0)

#define TOP_ELEVATORBOX_SIZE vec3(0.02, 0.05, 1.0)
#define TOP_ELEVATORBOX_POS vec3(1.4, -0.65, 2.0)
    
#define EXITDOOR_SIZE vec3(1.1, 2.95, 0.2)
#define EXITDOOR_POS vec3(3.2, 2.95, -24.0)

#define EXITDOOR_HSTROKE_SIZE vec3(1.1, 0.05, 0.05)
#define EXITDOOR_HSTROKE_POS vec3(3.4, 4.3, -24.0)
#define EXITDOOR_VSTROKE_SIZE vec3(0.07, 2.95, 0.05)
#define EXITDOOR_VSTROKE_POS vec3(2.2, 2.95, -24.0)

#define EXITDOOR_WINDOW_SIZE vec3(0.1, 0.5, 0.1)
#define EXITDOOR_WINDOW_POS vec3(2.75, 3.25, -24.2)
    
#define PLINTHE_SIZE vec3(4.15, 0.095, 14.5)
#define PLINTHE_POS vec3(0.0, 0.41, -9.5)

#define Square_SIZE vec3(0.55, 0.55, 0.2)
#define SquareL1_Front1 vec3(0.675, 4.6, -23.3)
#define SquareL1_Front2 vec3(-0.475, 4.6, -23.3)
#define SquareL1_Front3 vec3(-1.625, 4.6, -23.3)
#define SquareL1_Front4 vec3(-2.775, 4.6, -23.3)

#define SquareL2_Front1 vec3(0.675, 3.45,  -23.3)
#define SquareL2_Front2 vec3(-0.475, 3.45, -23.3)
#define SquareL2_Front3 vec3(-1.625, 3.45, -23.3)
#define SquareL2_Front4 vec3(-2.775, 3.45, -23.3)

#define SquareL3_Front1 vec3(0.675, 2.3,  -23.3)
#define SquareL3_Front2 vec3(-0.475, 2.3, -23.3)
#define SquareL3_Front3 vec3(-1.625, 2.3, -23.3)
#define SquareL3_Front4 vec3(-2.775, 2.3, -23.3)

#define SquareL4_Front1 vec3(0.675, 1.15,  -23.3)
#define SquareL4_Front2 vec3(-0.475, 1.15, -23.3)
#define SquareL4_Front3 vec3(-1.625, 1.15, -23.3)
#define SquareL4_Front4 vec3(-2.775, 1.15, -23.3)

//TOP squares
#define SquareTOP_SIZE vec3(0.6, 0.1, 0.6)    
#define SquareTOPL1_POS vec3(2.5, 0.01, 0.125)

//Materials
#define MarbleMatID 1.0
#define MarbleMatAlbedo vec3(0.78431, 0.78431, 0.82353)
#define MarbleMatRoughness 0.4
#define MarbleMatReflectivity 0.35
    
#define BlackMarbleMatID 2.0
#define BlackMarbleMatAlbedo vec3(0.35294, 0.34118, 0.40177)
#define BlackMarbleMatRoughness 0.35
#define BlackMarbleMatReflectivity 0.7
    
#define TopWhiteMatID 3.0
#define TopWhiteMatAlbedo vec3(0.9)
#define TopWhiteMatRoughness 0.175
#define TopWhiteMatReflectivity 0.5

#define BlackRoughMatID 4.0
#define BlackRoughMatAlbedo vec3(0.055, 0.06, 0.065)
#define BlackRoughMatRoughness 0.7
#define BlackRoughMatReflectivity 0.95

#define SquareOrangeMatID 5.0
#define SquareOrangeMatAlbedo vec3(1.0, 0.32549, 0.10196)

#define ElevBlackMatID 6.0
#define ElevBlackMatAlbedo vec3(0.025)
#define ElevBlackMatRoughness 0.9
#define ElevBlackMatReflectivity 0.45
    
#define MetalMatID 7.0
#define MetalMatAlbedo vec3(0.05)
#define MetalMatRoughness 0.9
#define MetalMatReflectivity 0.3

#define SquareWhiteMatID 8.0
#define SquareWhiteMatAlbedo vec3(1.0)

#define CeilMatAlbedo vec3(0.1)
#define CeilMatReflectivity 0.75

//Map scene-----------------------------------------------------
//Distance functions by Iq -------------------------------------
// http://www.iquilezles.org/www/articles/distfunctions/distfunctions.htm
// https://www.shadertoy.com/view/Xds3zN
//--------------------------------------------------------------
//Union
vec2 OperationUnion(vec2 dist1, vec2 dist2)
{
    return (dist1.x < dist2.x) ? dist1 : dist2;
}

float OpU(float dist1, float dist2)
{
	return min(dist1, dist2);
}

vec2 OperationSoustraction(vec2 dist1, vec2 dist2)
{
    if (-dist2.x > dist1.x)
        return vec2(-dist2.x, dist2.y);

    return dist1;
}

float OpS(float dist1, float dist2)
{
	return max(-dist2, dist1);
}

float OpI(float dist1, float dist2)
{
	return max(dist1, dist2);
}

vec3 OperationRepetition(vec3 mPos, vec3 rep)
{
    return mod(mPos, rep) - 0.5 * rep;
}

float UnsignedDistBox(vec3 mPos, vec3 boxSize)
{
  return length(max(abs(mPos) - boxSize, 0.0));
}

float SignedDistBox(vec3 mPos, vec3 boxSize)
{
  vec3 d = abs(mPos) - boxSize;
  return min(max(d.x, max(d.y, d.z)), 0.0) + length(max(d, 0.0));
}

float SignedDistSphere(vec3 mPos, float rad)
{
    return length(mPos)- rad;
}

//--------------------------------------------------------------

vec2 SceneDistance(vec3 mPos)
{
    vec2 room = vec2(-SignedDistBox(mPos - BOXROOM_POS, BOXROOM_SIZE), MarbleMatID);
    
	// Plinthe
    vec2 plinthe = vec2(SignedDistBox(mPos - PLINTHE_POS, PLINTHE_SIZE), BlackRoughMatID);
    vec2 res = OperationSoustraction(room, plinthe);

    // ************************************************ //
    // Kind of "plane tree" in order to avoid calculate //
    //          the useless parts of the scene          //
    // ************************************************ //
    // Right wall
    if(mPos.x < -3.9)
    {
        float exclusionBox = SignedDistBox(mPos - vec3(0.0, 2.9, -15.0), vec3(4.5, 2.6, 6.0));
        // Elevator box above
        float elevatorStuff = OpI(exclusionBox, SignedDistBox(OperationRepetition(mPos - TOP_ELEVATORBOX_POS,
                                                                                  vec3(11.0, 10.0, 3.5)),
                                                              TOP_ELEVATORBOX_SIZE));
        // Elevator call button
        elevatorStuff = OpU(elevatorStuff, OpI(exclusionBox, 
                                               SignedDistBox(OperationRepetition(mPos - vec3(1.4, -3.0, 4.0),
                                                                                 vec3(11.0, 11.0, 3.5)),
                                                             vec3(0.05, 0.5, 0.125))));

        res = OperationUnion(res, vec2(elevatorStuff, MetalMatID));

        // Elevator floors
        float elevatorFloor = OpI(exclusionBox, SignedDistBox(OperationRepetition(mPos - vec3(1.4, -0.75, 2.0),
                                                                                  vec3(11.0, 11.0, 3.5)),
                                                              vec3(0.01, 0.15, 0.3)));
        res = OperationUnion(res, vec2(elevatorFloor, ElevBlackMatID));

        float elev = OpI(exclusionBox, SignedDistBox(OperationRepetition(mPos - ELEVATOR_POS,
                                                                         vec3(11.5, 10.0, 3.5)),
                                                     ELEVATOR_SIZE));
        res = OperationSoustraction(res, vec2(elev, ElevBlackMatID));
    }
    // Ground
   	else if(mPos.y < 0.4)
    {
        // Squares on ground
        float exclusionBoxGround = SignedDistBox(mPos - vec3(0.0, 0.2, -9.5), vec3(3.65, 0.4, 14.5));
        float squareGround = OpI(exclusionBoxGround, 
                              UnsignedDistBox(OperationRepetition(mPos - vec3(0.0, 5.21, 0.25), 
                                                                  vec3(5.0, 10.0, 1.5)),
                                              vec3(3.9, 0.1, 0.5)));
        res = OperationUnion(res, vec2(squareGround, BlackMarbleMatID));
    }
    // Top
    else if(mPos.y > 5.1)
    {
        // Squares top
        float exclusionBoxCeil = SignedDistBox(mPos - vec3(0.0, 5.5, -9.5), vec3(3.65, 0.4, 14.5));
        float squareTop = OpI(exclusionBoxCeil, 
                              UnsignedDistBox(OperationRepetition(mPos - SquareTOPL1_POS, 
                                                                  vec3(1.25, 11.15, 1.3)),
                                              SquareTOP_SIZE));

        //Adding small bump distortion with Sin and Cos
        res = OperationUnion(res, vec2(squareTop + ((sin(mPos.x * 12.5) + cos(mPos.z * 7.5))* 0.0005), TopWhiteMatID));


        // Black spheres
        float blackCylinder = OpI(exclusionBoxCeil, 
                                  SignedDistSphere(OperationRepetition(mPos - vec3(7.1, 1.575, 1.3),
                                                                         vec3(8.0, 8.0, 3.8)),
                                                     0.15));

        res = OperationUnion(res, vec2(blackCylinder, BlackRoughMatID));
    }
    // Front
    else if(mPos.z < -19.0)
    {
        // Exit door
        float exitDoor = SignedDistBox(mPos - EXITDOOR_POS, EXITDOOR_SIZE);
        res = OperationSoustraction(res, vec2(exitDoor, BlackRoughMatID));

        float exitDoorStroke = SignedDistBox(mPos - EXITDOOR_HSTROKE_POS, EXITDOOR_HSTROKE_SIZE);
        exitDoorStroke = OpU(exitDoorStroke, SignedDistBox(mPos - EXITDOOR_VSTROKE_POS, 
                                                           EXITDOOR_VSTROKE_SIZE));

        res = OperationUnion(res, vec2(exitDoorStroke, BlackRoughMatID));

        // Window
        res = OperationSoustraction(res, vec2(SignedDistBox(mPos - EXITDOOR_WINDOW_POS, 
                                                              EXITDOOR_WINDOW_SIZE), SquareWhiteMatID));

        // Door handle SignedDistSphere
        res = OperationUnion(res, vec2(SignedDistSphere(mPos - vec3(2.45, 2.2, -24.2), 0.125)
                                         , MetalMatID));

        // Squares front
        float frontWhiteSquares = UnsignedDistBox(mPos - SquareL1_Front1, Square_SIZE);
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL1_Front2, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL1_Front4, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL2_Front2, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL2_Front3, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL2_Front4, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL3_Front1, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL3_Front2, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL3_Front4, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL4_Front2, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL4_Front3, Square_SIZE));
        frontWhiteSquares = min(frontWhiteSquares, UnsignedDistBox(mPos - SquareL4_Front4, Square_SIZE));

        res = OperationUnion(res, vec2(frontWhiteSquares, SquareWhiteMatID));

        float fOrSqr = UnsignedDistBox(mPos - SquareL1_Front3, Square_SIZE);
        fOrSqr = OpU(fOrSqr, UnsignedDistBox(mPos - SquareL2_Front1, Square_SIZE));
        fOrSqr = OpU(fOrSqr, UnsignedDistBox(mPos - SquareL4_Front1, Square_SIZE));

        res = OperationUnion(res, vec2(fOrSqr, SquareOrangeMatID));

        res = OperationUnion(res, vec2(UnsignedDistBox(mPos - SquareL3_Front3, 
                                                                     Square_SIZE), BlackRoughMatID));
    }
    
    return res;
}
//--------------------------------------------------------------

//Calculate normals--------------------------------------------
vec3 CalcNormal(vec3 mPos)
{
	vec3 eps = vec3(0.025, 0.0, 0.0);
	vec3 nor = vec3(
	    SceneDistance(mPos + eps.xyy).x - SceneDistance(mPos - eps.xyy).x,
	    SceneDistance(mPos + eps.yxy).x - SceneDistance(mPos - eps.yxy).x,
	    SceneDistance(mPos + eps.yyx).x - SceneDistance(mPos - eps.yyx).x);
		
	return normalize(nor);
}
//-------------------------------------------------------------

//Calculate AO-------------------------------------------------
float CalcAO(vec3 mPos, vec3 nor)
{
	float totAO = 0.0;
    float sca = 1.0;
    for(int aoi = 0; aoi < 3; aoi++)
    {
        float hr = 0.01 + 0.025 * float(aoi + 2);
        vec3 aoPos =  nor * hr + mPos;
        float dd = SceneDistance(aoPos).x;
        totAO += -(dd-hr)*sca;
        sca *= 0.75;
    }
    return clamp(1.0 - 4.0 * totAO, 0.0, 1.0);
}
//-------------------------------------------------------------

//-------------------------------------------------------------
// Shading ----------------------------------------------------
//-------------------------------------------------------------
vec3 Shading(vec3 rayOrigin, vec3 rayDirection,
               rayIntersect mIntersection, lightTube lig)
{
	vec3 L0 = lig.tubeStart - mIntersection.mPos;
	vec3 L1 = lig.tubeEnd - mIntersection.mPos;
	vec3 r = reflect(-rayDirection, mIntersection.nor);
    
	vec3 Ld	= L1 - L0;
	float RoL0 = dot(vec3(r), L0);
	float RoLd = dot(vec3(r), Ld);
	float L0oLd = dot(L0, Ld);
	float distLd = length(Ld);
	float t = (RoL0 * RoLd - L0oLd) / (distLd * distLd - RoLd * RoLd);
	vec3 closestPoint = L0 + Ld * clamp(t, 0.0, 1.0);
    
	vec3 LightDir = normalize(closestPoint);
	
    float NdotV = clamp(dot(mIntersection.nor, -rayDirection), 0.0, 1.0);
	float NdotL = clamp(dot(mIntersection.nor, LightDir), 0.0, 1.0);
    
	//ambient, AO, BRDF
    float ao = CalcAO(mIntersection.mPos, mIntersection.nor);
	vec3 amb = 0.3 * clamp(0.5 + 0.5 * mIntersection.nor.y, 0.0, 1.0) * AMBIENT_COLOR * ao;

	//Diffuse
	vec3 diff = amb + mIntersection.mat.albedo - mIntersection.mat.reflectivity / (2.0 * PI);
	vec3 colorResult = diff * NdotL;
	
	if (NdotV * NdotL > 0.0)
	{
		//Half angle Vector
		vec3 HalfAngleV = normalize(-rayDirection + LightDir);
		float NdotH = clamp(dot(mIntersection.nor, HalfAngleV), 0.0, 1.0);
		float VdotH = clamp(dot(-rayDirection, HalfAngleV), 0.0, 1.0);
        
		// R0 good air-dielectric interface for 1.4 < Nt < 2.2
        // Variances in micro-surfaces will result in a brighter or dimmer fresnel
        // Fresnel Schlick's approximation
		float F = pow(1.0 - VdotH, 5.0);
        
		// Cook-Torrance Geometry function
		float NH2 = 2.0 * NdotH / VdotH;
		float G = min(1.0, min(NdotV, NdotL) * NH2);

		// Roughness Beckmann Distribution
		float sq_NdotH   = NdotH * NdotH;
		float sq_NdotH_M = sq_NdotH * (mIntersection.mat.roughness * mIntersection.mat.roughness);
		float D = exp((sq_NdotH - 1.0) / sq_NdotH_M) / (PI * sq_NdotH * sq_NdotH_M);

		//float brdf_spec = (F * D * G) / (NdotV * NdotL * 4.0);
        float brdf_spec = (F * D * G) / 4.0;
		colorResult += (1.0 - mIntersection.mat.reflectivity) * brdf_spec;
    }
	return colorResult * lig.color * ao;
}
//-------------------------------------------------------------

//RaymarchScene-------------------------------------------------
vec3 RayMarchScene(vec3 rayOrigin, vec3 rayDirection,
                   inout rayIntersect mIntersection)
{
    float dist = EPSILON * 2.0;
    float t = 0.5;
    float m = -1.0;
    
    for(int i = 0; i < 45; i++)
    {
        if(abs(dist) < EPSILON || t > FARCLIP) 
            break;

        t += dist;
        vec2 res = SceneDistance(rayOrigin + rayDirection * t);
        dist = res.x;
	    m = res.y;
    }
    
    if(t > FARCLIP)
        m = -1.0;
    
    vec3 pos = rayOrigin + t * rayDirection;
    mIntersection.mPos = pos;
    mIntersection.nor = CalcNormal(pos);
    mIntersection.dist = t;
    
	vec3 colorResult = vec3(0.0);
	if (t > EPSILON)
	{
		material mat;
		mat.albedo = SquareWhiteMatAlbedo;
		mat.roughness = 0.5;
		mat.reflectivity = 0.7;
		
        if(m - 0.5 <= MarbleMatID)
        {
            if(mIntersection.nor.z > 0.0)
            {
                mat.albedo = texture2D(iChannel0, (pos.xy)/5.0, -100.0).xxx * BlackMarbleMatAlbedo - 0.25;
                mat.roughness = BlackMarbleMatRoughness;
                mat.reflectivity = BlackMarbleMatReflectivity;
            }
            else if(mIntersection.nor.y < 0.0)
            {
                mat.albedo = CeilMatAlbedo;
                mat.reflectivity = CeilMatReflectivity;
            }
            else
            {
                float r = texture2D(iChannel0, (pos.zy - pos.yx)/7.0, -100.0).x;
                mat.albedo = r * MarbleMatAlbedo;
                mat.roughness = MarbleMatRoughness;
                mat.reflectivity = MarbleMatReflectivity + min(r * r, 0.3);
            }
        }
        else if(m - 0.5 <= BlackMarbleMatID)
        {
            mat.albedo = texture2D(iChannel0, (pos.xz)/4.0, -100.0).xxx * BlackMarbleMatAlbedo - 0.15;
			mat.roughness = BlackMarbleMatRoughness;
			mat.reflectivity = BlackMarbleMatReflectivity;
        }
        else if(m - 0.5 <= TopWhiteMatID)
        {
            mat.albedo = TopWhiteMatAlbedo;
			mat.roughness = TopWhiteMatRoughness;
			mat.reflectivity = TopWhiteMatReflectivity;
        }
        else if(m - 0.5 <= BlackRoughMatID)
        {
            mat.albedo = BlackRoughMatAlbedo;
			mat.roughness = BlackRoughMatRoughness;
			mat.reflectivity = BlackRoughMatReflectivity;
        }
		else if(m - 0.5 <= SquareOrangeMatID)
		{
			mat.albedo = SquareOrangeMatAlbedo;
		}
        else if(m - 0.5 <= ElevBlackMatID)
		{
			mat.albedo = ElevBlackMatAlbedo;
			mat.roughness = ElevBlackMatRoughness;
			mat.reflectivity = ElevBlackMatReflectivity;
		}
        else if(m - 0.5 <= MetalMatID)
        {
            mat.albedo = MetalMatAlbedo;
			mat.roughness = MetalMatRoughness;
			mat.reflectivity = MetalMatReflectivity;
        }


		
		mIntersection.mat = mat;
		
		//Shading / lighting
        lightTube lig;
		lig.tubeStart = vec3(0.675, 4.7, -22.8);
        lig.tubeEnd = vec3(0.675, 1.25, -22.8);
        lig.color = vec3(1.0, 0.9451, 0.92157);
        
        for (int i = 0; i < 4; i++)
        {
			colorResult += 0.5 * Shading(rayOrigin, rayDirection, mIntersection, lig);
            lig.tubeStart.x -= 1.15;
            lig.tubeEnd.x -= 1.15;
        }
	}
    return colorResult;
}
//--------------------------------------------------------------

//Render the scene----------------------------------------------
vec3 RenderScene(vec3 rayOrigin, vec3 rayDirection)
{ 
	rayIntersect mIntersection;
	mIntersection.mPos = vec3(0.0);
	mIntersection.nor = vec3(0.0);
	mIntersection.dist = 0.0;
	
	// Opaque
    vec3 accum = RayMarchScene(rayOrigin, rayDirection, mIntersection);
    
    //reflection
	rayIntersect mIntersecReflect = mIntersection;
	vec3 rayDirReflect = reflect(rayDirection, mIntersecReflect.nor);
	vec3 rayOriginReflect = mIntersecReflect.mPos + EPSILON * rayDirReflect;
	float refl = 1.0;

	for (float k = 1.0; k < 3.0; ++k)
	{
		mIntersecReflect.dist = -1.0;
		refl *= max(0.75 - mIntersection.mat.reflectivity, 0.0);
		accum += RayMarchScene(rayOriginReflect, rayDirReflect, mIntersecReflect) * refl;

		if ((mIntersecReflect.dist < EPSILON)) 
			break;
		
		rayOriginReflect = mIntersecReflect.mPos;
		rayDirReflect = reflect(rayDirReflect, mIntersecReflect.nor);
		rayOriginReflect += EPSILON * rayDirReflect;
	}
    
	return clamp(accum, 0.0, 1.0);
}
//--------------------------------------------------------------
//END THE SCENE-------------------------------------------------

//Post Process ------------------------------------------------
vec3 GammaCorrection(vec3 colorResult)
{
	return pow(colorResult, vec3(0.45455));
}

vec3 Contrast(vec3 colorResult)
{
    return colorResult * 0.6 + 0.4 * colorResult * colorResult * (3.0 - 2.0 * colorResult);
}

vec3 Tint(vec3 colorResult)
{
    return colorResult * vec3(1.025, 1.02, 1.0);
}

vec3 Vigneting(vec3 colorResult)
{
	vec2 inXYPos = (gl_FragCoord.xy / iResolution.xy);
	return colorResult * (0.55 + 0.45 * pow(70.0 * inXYPos.x * inXYPos.y * (1.0 - inXYPos.x) * (1.0 - inXYPos.y), 0.15));
}

void ApplyPostProcess(inout vec3 colorResult)
{
    colorResult = GammaCorrection(colorResult);
    colorResult = Contrast(colorResult);
    colorResult = Tint(colorResult);
    colorResult = Vigneting(colorResult);
}
//-------------------------------------------------------------

//Post CameraDirection -----------------------------------------
vec3 GetCameraRayDir(vec2 mUV, in vec3 camPosition, vec3 camTarget)
{
	vec3 forwardVector = normalize(camTarget - camPosition);
	vec3 rightVector = normalize(cross(vec3(0.0, 1.0, 0.0), forwardVector));
	vec3 upVector = normalize(cross(forwardVector, rightVector));
    
	vec3 camDirection = normalize(	mUV.x * rightVector 
                                  +	mUV.y * upVector
                                  +	3.3 * forwardVector);
    
	return camDirection;
}
//--------------------------------------------------------------

//Screen coordinates ------------------------------------------
vec2 GetScreenSpaceCoord()
{
	vec2 mUV = (gl_FragCoord.xy/iResolution.xy) * 2.0 - 1.0;
	mUV.x *= iResolution.x / iResolution.y;

	return mUV;	
}
//-------------------------------------------------------------

//Main loop-----------------------------------------------------
void main(void)
{
    vec2 mUV = GetScreenSpaceCoord();

    //Camera --------------------------------------------------------
    float mTime = sin(iGlobalTime / 10.0) * 0.1;
    vec3 rayOrigin = vec3(-mTime, 3.0, 5.0);
    vec3 camTarget = vec3(mTime, 2.95, 0.0);
    
    vec3 rayDirection = GetCameraRayDir(mUV, rayOrigin, camTarget);
    //END CAMERA ----------------------------------------------------

    vec3 mColor = RenderScene(rayOrigin, rayDirection);
    
    ApplyPostProcess(mColor);

    gl_FragColor = vec4(mColor, 1.0);
}
//--------------------------------------------------------------
