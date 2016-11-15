// https://www.shadertoy.com/view/Xds3Dn
//------------------------------------------------------------------------------------------
//
// Implicit curve segment example - @P_Malin
//
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.
//
//------------------------------------------------------------------------------------------
//
// I made this shader to explain the curve segment code in my shadertoy font shader:
//
//	 https://www.shadertoy.com/view/lslGDn
//
// Click and drag the mouse to move the control points
//
//------------------------------------------------------------------------------------------
//
// GPU Gems 3 has an article explaining something much better:
// http://http.developer.nvidia.com/GPUGems3/gpugems3_ch25.html
//
// There is some more advanced work in this paper:
// "Random-access rendering of general vector graphics"
// http://research.microsoft.com/en-us/um/people/hoppe/proj/ravg/
//
// Also see more examples here:
// http://glsl.heroku.com/e#4976.0
// http://glsl.heroku.com/e#4978.0
// http://glsl.heroku.com/e#5007
//
//------------------------------------------------------------------------------------------


//#define QUADRATIC_CURVE
//#define FULLSCREEN_CURVE

vec3 cCurveColour = vec3(0.0, 0.4, 0.9);
vec3 cControlPointColour = vec3(1.0, 0.5, 0.75);

vec3 cBackground1Colour = vec3(226.0, 255.0, 225.0) / 255.0;
vec3 cBackground2Colour = vec3(232.0, 253.0, 255.0) / 255.0;

float PointAboveCurve(vec2 uv)
{
	#ifdef QUADRATIC_CURVE
		// This is the more common curve to use for this sort of thing
		uv.x = 1.0 - uv.x;
		return uv.y - uv.x * uv.x;
	#else
		// This is the curve I used in the "shadertoy" font shader
		// It is quarter of a circle. It solved some problems for me compared to
		// the above curve as the curve ends up much closer to the control point.
		uv = 1.0 - uv;
		return 1.0 - dot(uv, uv);
	#endif
}

float PointOutsideTriangle(vec2 uv)
{
	if(uv.x < 0.0)
		return 1.0;

	if(uv.y < 0.0)
		return 1.0;

	if(uv.x + uv.y > 1.0)
		return 1.0;
	
	return 0.0;
}

// 2D Cross product
float Cross( const in vec2 A, const in vec2 B )
{
    return A.x * B.y - A.y * B.x;
}

// Returns the barycentric co-ordinates of point P in the triangle
vec2 GetUV(const in vec2 A, const in vec2 B, const in vec2 C, const in vec2 P)
{
    float f1 = Cross(A-B, A-P);
    float f2 = Cross(B-C, B-P);
    float f3 = Cross(C-A, C-P);
    
    return vec2(f1, f2) / (f1 + f2 + f3);
}

float DrawPoint(vec2 vPos, vec2 vUV)
{
	vec2 vDelta = (vPos - vUV) * iResolution.xy;
	float fDist = length(vDelta);
	return smoothstep(7.0, 5.0, fDist);
}

void main(void)
{
	vec2 vScreenUV = gl_FragCoord.xy / iResolution.xy;
	
	// Calculate the control point co-ordiantes
	
	vec2 A= vec2(0.1, 0.2);
	vec2 B= vec2(0.5, 0.5);
	vec2 C= vec2(0.9, 0.2);
	
	if(iMouse.x > 0.0)
	{
		B = iMouse.xy / iResolution.xy;
		C = abs(iMouse.zw) / iResolution.xy;
	}

	#ifdef FULLSCREEN_CURVE
		A = vec2(0.0, 1.0);
		B = vec2(0.0, 0.0);
		C = vec2(1.0, 0.0);
	#endif

	vec2 BAdjusted = B;	

	// Figure out where the corner of triangle should be
	#ifndef QUADRATIC_CURVE
		BAdjusted += (B - (A + C) * 0.5) * sqrt(2.0);
	#else
		BAdjusted += (B - (A + C) * 0.5) * (1.0 + sqrt(5.0));
	#endif
	
	// Get the UV co-ordiantes in the control point triangle	
	vec2 vCurveUV = GetUV(A, BAdjusted, C, vScreenUV);
	
	// See if the we are above the curve at this UV co-ordiante
	float fPointAboveCurve = PointAboveCurve(vCurveUV);
	
	// Flip the sign of the curve test if we have a "concave" curve segment
	float isConcave = Cross(A-B, C-B);
	if(isConcave < 0.0)
	{
		fPointAboveCurve = -fPointAboveCurve;
	}
		
	float fOutsideTriangle = PointOutsideTriangle(vCurveUV);

	// Pick a colour based on the curve and triangle
	
	vec3 vResult = vec3(0.0);
	if(fOutsideTriangle > 0.0)
	{
		if(fPointAboveCurve > 0.0)
		{
			vResult = cBackground1Colour;
		}
		else
		{
			vResult = cBackground2Colour;
		}
	}
	else
	{
		if(fPointAboveCurve > 0.0)
		{
			vResult = cCurveColour;
		}
		else
		{
			vResult = cBackground1Colour;
		}
	}
	
	// Draw control Points	
	
	vResult = mix( vResult, cControlPointColour, DrawPoint(A, vScreenUV));
	vResult = mix( vResult, cControlPointColour, DrawPoint(B, vScreenUV));
	vResult = mix( vResult, cControlPointColour, DrawPoint(BAdjusted, vScreenUV));
	vResult = mix( vResult, cControlPointColour, DrawPoint(C, vScreenUV));	
	
	gl_FragColor = vec4(vResult,1.0);
}
