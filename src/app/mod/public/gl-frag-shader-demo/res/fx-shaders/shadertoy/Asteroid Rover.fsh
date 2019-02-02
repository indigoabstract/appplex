// https://www.shadertoy.com/view/Mds3DX

// iChannel0: t11
// iChannel1: t2
// iChannel2: t6

/*

TODO:

* clean stuff up, move various params to "global / uniform" space

* put back in the "early out" for if the ray leaves the bounding box of the terrain

* maybe have some haze at the horizon?

* cone light instead of directional?


*/

const float c_timeMultiplier = 0.25;
float scaledTime = iGlobalTime * c_timeMultiplier;

const float c_maxVisibilityDistance = 7.0;

float hash(float n) {
    return fract(sin(n) * 43758.5453123);
}

// Value noise generator. Returns
// three values on [-1, +1]
vec3 noised(vec2 x) {
    vec2 p = floor(x);
    vec2 f = fract(x);

	// The constant for tileWidth doesn't matter much unless it is too small
    const float tileWidth = 1024.0;
    float n = p.x + p.y * tileWidth;

    // Grab noise values at four corners of a square
    float a = hash(n +  0.0);
    float b = hash(n +  1.0);
    float c = hash(n + tileWidth);
    float d = hash(n + tileWidth + 1.0);

    // use smoothstep-filtered lerp for one component and compute the derivatives for the others
	// See http://www.iquilezles.org/www/articles/morenoise/morenoise.htm

    // The (negative) smoothstep weight
    vec2 u = f * f * (3.0 - 2.0 * f);
	return vec3(a+(b-a)*u.x+(c-a)*u.y+(a-b-c+d)*u.x*u.y,
				30.0*f*f*(f*(f-2.0)+1.0)*(vec2(b-a,c-a)+(a-b-c+d)*u.yx));
}

const int octaves = 3;

// On the range [0, 1].  This is the sum
// of a convergent series http://en.wikipedia.org/wiki/Series_(mathematics)#Convergent_series, 
// where each term has a pseudorandom weight on [-1, 1].  The largest sum is therefore
// 2 (the smallest is -2), and the final line of code rescales this to the unit interval.
// 
float HeightAtPos(vec2 P) {
	
	P *= 0.5;
	
    const mat2 M2 = mat2(1.6, -1.2, 1.2, 1.6);
    float height = 0.0;
	vec2 d = vec2(0.0);

    // Magnitude at this octave
    float magnitude = 1.0;

    // Add multiple octaves of noise, chosen from points that spiral outward
    // to avoid hitting the tiling period of the noise function.
    for (int i = 0; i < octaves; ++i) {
        vec3 n = noised(P);
        d += n.yz;

        // The 1 + |d|^2 denominator creates the mountainous lumpiness.
        // Without it, this is a standard value noise function.
        height += magnitude * n.x / (1.0 + dot(d, d));
        P = M2 * P;
		magnitude *= 0.5;
    }

	// iq's original had 0.5 here, but that doesn't fit the range
    return (height * 0.5 + 0.5);
}

//=======================================================================================
vec3 NormalAtPos( vec2 p )
{
	float eps = 0.01;
    vec3 n = vec3( HeightAtPos(vec2(p.x-eps,p.y)) - HeightAtPos(vec2(p.x+eps,p.y)),
                         2.0*eps,
                         HeightAtPos(vec2(p.x,p.y-eps)) - HeightAtPos(vec2(p.x,p.y+eps)));
    return normalize( n );
}

//=======================================================================================
vec2 path( float time ) {
	return vec2( cos(0.2+0.0525*time), sin(0.1+0.07425*time));	
}

//=======================================================================================
vec3 CameraPointInTime (float t)
{
	vec2 pos2D = path(t);
	pos2D *= 16.0;
	return vec3(pos2D.x, HeightAtPos(pos2D) + 0.2, pos2D.y);
}

//=======================================================================================
// Scene parameters
//=======================================================================================

float cameraTime1 = scaledTime;

vec3 cameraPos = CameraPointInTime(cameraTime1);
vec3 cameraAt = CameraPointInTime(cameraTime1 + 0.01);

vec3 cameraUp = mix(NormalAtPos(cameraPos.xz),vec3(0.0,1.0,0.0),0.75);
vec3 cameraFwd2 = normalize(cameraAt - cameraPos);
vec3 cameraLeft  = normalize(cross(cameraFwd2, cameraUp));
vec3 cameraFwd = normalize(cross(cameraUp, cameraLeft));

float cameraViewWidth	= 6.0;
float cameraViewHeight	= cameraViewWidth * iResolution.y / iResolution.x;
float cameraDistance	= 6.0;  // intuitively backwards!

vec2 minimapTarget = ((path(cameraTime1) + 1.0) * 0.1 + vec2(0.0,0.8)) * iResolution.xy;

//=======================================================================================
vec3 ShadePoint (in vec3 pos, float time)
{
	/*
	bool whiteSquare = true;
	
	if (mod(pos.x, 2.0) < 1.0)
		whiteSquare = !whiteSquare;

	if (mod(pos.z, 2.0) < 1.0)
		whiteSquare = !whiteSquare;	
	
	vec3 diffuseColor = whiteSquare ? vec3(1.0,1.0,1.0) : vec3(0.4,0.4,0.4);
	*/
	
	vec3 color1 = texture2D( iChannel2, pos.xz ).rgb;
	vec3 color2 = texture2D( iChannel1, pos.xz ).rgb;
	
	vec3 diffuseColor = mix(color1, color2, pos.y * 2.0);
	
	vec3 reverseLightDir = normalize(vec3(1.0,1.0,-1.0));
	vec3 lightColor = vec3(1.0,1.0,1.0);	
	vec3 ambientColor = vec3(0.1,0.1,0.1);

	vec3 normal = NormalAtPos(pos.xz);

	vec3 color = ambientColor;
	float dp = dot(normal, reverseLightDir);
	if(dp > 0.0)
		color += (diffuseColor * dp * lightColor);	
	
	float b = 0.5;
    float fogAmount = pow(time / c_maxVisibilityDistance, 1.0);
    return mix( color, vec3(0.0,0.0,0.0), fogAmount );	
}

//=======================================================================================
bool HandleRay (in vec3 rayPos, in vec3 rayDir, inout vec3 pixelColor)
{
	float time = 0.0;
	float lastHeight = 0.0;
	float lastY = 0.0;
	float deltaT = 0.01;
	vec3 pos;
	float height;
	bool hitFound = false;
	for (int index = 0; index < 150; ++index)
	{		
		pos = rayPos + rayDir * time;
		height = HeightAtPos(pos.xz);
		if (height > pos.y)
		{					
			hitFound = true;
			break;
		}
		
		deltaT = max(0.01*float(time), (pos.y - height)*0.5);
		time += deltaT;		
		
		if (time > c_maxVisibilityDistance)
			break;
		
		lastHeight = height;
		lastY = pos.y;
	}
	
	if (hitFound) {
		time = time - deltaT + deltaT*(lastHeight-lastY)/(pos.y-lastY-height+lastHeight);
		pos = rayPos + rayDir * time;
		height = HeightAtPos(pos.xz);		
		pixelColor = ShadePoint(vec3(pos.x, height, pos.z), time);
	}
	
	return hitFound;
}

//=======================================================================================
bool DrawUI (inout vec3 pixelColor)
{
	
	// early out of UI box area
	int borderX = int(iResolution.x * 0.2) + 1;
	int borderY = int(iResolution.y * 0.8) - 1;

	int bufferX = int(iResolution.x * 0.05);
	int bufferY = int(iResolution.y * 0.05);

	int pixelX = int(gl_FragCoord.x) - bufferX;
	int pixelY = int(gl_FragCoord.y) + bufferY;	

	if (pixelX < 0 || pixelX > borderX || pixelY > int(iResolution.y) || pixelY < borderY)
		return false;
	
	if (pixelX == 0 || pixelX == borderX ||
		pixelY == int(iResolution.y) || pixelY == borderY)
	{
		pixelColor += vec3(0.0,1.0,0.0);
		return true;
	}
	
	if (int(minimapTarget.x)+1 == pixelX)
	{
		pixelColor += vec3(1.0,0.0,0.0);
		return true;
	}
	if (int(minimapTarget.y)-1 == pixelY)
	{
		pixelColor += vec3(1.0,1.0,0.0);
		return true;
	}
	
	vec2 dist = vec2(minimapTarget.x + 1.0 - float(pixelX), minimapTarget.y - 1.0 - float(pixelY));
	if (length(dist) < float(bufferX) * 0.15)
	{
		pixelColor += vec3((sin(scaledTime*50.0)+ 1.0) * 0.5,0.0,0.0);
		return true;		
	}	
	
	if (mod(float(pixelX), float(bufferX) * 0.5) < 0.1)
	{
		pixelColor += vec3(0.0,0.2,0.0);
		return true;
	}	
	
	if (mod(float(pixelY+2), float(bufferY) * 0.5) < 0.1)
	{
		pixelColor += vec3(0.0,0.2,0.0);
		return true;
	}		
	
	return false;
}

//=======================================================================================
void main (void)
{		
	
	// UI
	vec3 pixelColor = vec3(0.0,0.0,0.0);	
	if (DrawUI(pixelColor))
	{
		gl_FragColor = vec4(pixelColor, 1.0);
		return;
	}
		
	// Terrain
	vec2 rawPercent = (gl_FragCoord.xy / iResolution.xy);
	vec2 percent = rawPercent - vec2(0.5,0.5);
	
	vec3 rayTarget = (cameraFwd * vec3(cameraDistance,cameraDistance,cameraDistance))
				   - (cameraLeft * percent.x * cameraViewWidth)
		           + (cameraUp * percent.y * cameraViewHeight);
	vec3 rayDir = normalize(rayTarget);
	
	
	if (HandleRay(cameraPos, rayDir, pixelColor))
	{
		gl_FragColor = vec4(pixelColor, 1.0);
		return;
	}
	
	// Stars
	float starXf = (atan(rayDir.x, rayDir.z) + 3.14) / 6.28;
	float starYf = (asin(rayDir.y) / 1.57);
	int starX = int(starXf * 1000.0 * 5.0);
	int starY = int(starYf * 250.0 * 5.0);
	
	
	float starTest = float(7 + starX * starY * 13);
	float value = abs(mod(starTest, 5000.0));
	if ( value >= 0.0 && value <= 1.0)
	{
		gl_FragColor = vec4(vec3(value * 0.5 + 0.5), 1.0);
		return;		
	}
	gl_FragColor = vec4(0.0,0.0,0.0, 1.0);
}
