// https://www.shadertoy.com/view/XdXGRs

// iChannel0: t14

// Ben Weston - 16/08/2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// constants, don't edit
const float tau = 6.28318530717958647692;
float Noise( in vec3 x );



// ------- EDIT THESE THINGS! -------

// Camera (also rotated by mouse)
const vec3 CamPos = vec3(0,20.0,-20.0);
const vec3 CamLook = vec3(0,0,0);

// Lights
const vec3 lightDir = vec3(0,1,1); // used for shadow rays
const vec3 fillLightDir = vec3(0,0,-1);
const vec3 lightColour = vec3(1,1,1);
const vec3 fillLightColour = vec3(.05,.15,.25);

// Shape
// This should return continuous positive values when outside and negative values inside,
// which roughly indicate the distance of the nearest surface.
float Isosurface( vec3 pos )
{
	float f = pos.y*.2-1.5;

	pos.z += iGlobalTime*.5;
	pos /= 2.0;
	f += Noise(pos/3.0)*3.0; // I tried putting this in a for loop but it vanished. Faster like this anyway
	f += Noise(pos/1.0)*1.0;
	f += Noise(pos*3.0)/3.0;
	//f += Noise(pos*9.0)/9.0;
	//f += Noise(pos*27.0)/27.0;
	
	return f;
}

// Colour
vec3 Shading( vec3 pos, vec3 norm, float shadow )
{
	vec3 albedo = mix( vec3(1,.8,.7), vec3(.5,.3,.2), Noise(pos*vec3(1,10,1)) );

	vec3 l = shadow*lightColour*max(0.0,dot(norm,lightDir));
	vec3 fl = fillLightColour*(dot(norm,fillLightDir)*.5+.5);
	
	return albedo*(l+fl);
}


// Precision controls
const float epsilon = .003;
const float normalPrecision = .1;
const float shadowOffset = .1;
const int traceDepth = 100; // takes time
const float drawDistance = 100.0;



// ------- BACK-END CODE -------

float Noise( in vec3 x )
{
    vec3 p = floor(x.xzy);
    vec3 f = fract(x.xzy);
	f = f*f*(3.0-2.0*f);
//	vec3 f2 = f*f; f = f*f2*(10.0-15.0*f+6.0*f2);

//cracks cause a an artefact in normal, of course
	
	// there's an artefact because the y channel almost, but not exactly, matches the r channel shifted (37,17)
	// this artefact doesn't seem to show up in chrome, so I suspect firefox uses different texture compression.
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel0, (uv+0.5)/256.0, -100.0 ).ba;
	return mix( rg.y, rg.x, f.z );
}

float Trace( vec3 ro, vec3 rd )
{
	float t = 0.0;
	float dist = 1.0;
	for ( int i=0; i < traceDepth; i++ )
	{
		if ( abs(dist) < epsilon || t > drawDistance || t < 0.0 )
			continue;
		dist = Isosurface( ro+rd*t );
		t = t+dist;
	}
	
	return t;//vec4(ro+rd*t,dist);
}

// get normal
vec3 GetNormal( vec3 pos )
{
	const vec2 delta = vec2(normalPrecision, 0);
	
	vec3 n;

// it's important this is centred on the pos, it fixes a lot of errors
	n.x = Isosurface( pos + delta.xyy ) - Isosurface( pos - delta.xyy );
	n.y = Isosurface( pos + delta.yxy ) - Isosurface( pos - delta.yxy );
	n.z = Isosurface( pos + delta.yyx ) - Isosurface( pos - delta.yyx );
	return normalize(n);
}				

// camera function by TekF
// compute ray from camera parameters
vec3 GetRay( vec3 dir, float zoom, vec2 uv )
{
	uv = uv - .5;
	uv.x *= iResolution.x/iResolution.y;
	
	dir = zoom*normalize(dir);
	vec3 right = normalize(cross(vec3(0,1,0),dir));
	vec3 up = normalize(cross(dir,right));
	
	return dir + right*uv.x + up*uv.y;
}

void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;

	vec3 camPos = CamPos;
	vec3 camLook = CamLook;

	vec2 camRot = tau*(vec2(.3,-.05)+vec2(.5,.25)*(iMouse.xy-iResolution.xy*.5)/iResolution.x);
	camPos.yz = cos(camRot.y)*camPos.yz + sin(camRot.y)*camPos.zy*vec2(-1,1);
	camPos.xz = cos(camRot.x)*camPos.xz + sin(camRot.x)*camPos.zx*vec2(1,-1);
	
	if ( Isosurface(camPos) <= 0.0 )
	{
		// camera inside ground
		gl_FragColor = vec4(0,0,0,0);
		return;
	}

	vec3 ro = camPos;
	vec3 rd;
	rd = GetRay( camLook-camPos, 1.0, uv );
	
	float t = Trace(ro,rd);

	vec3 result = vec3(.7,.9,1.2);
	if ( t > 0.0 && t < drawDistance )
	{
		vec3 pos = ro+t*rd;
			
		vec3 norm = GetNormal(pos);
		
		// shadow test
		float shadow = 1.0;
		if ( Trace( pos+lightDir*shadowOffset, lightDir ) < drawDistance )
			shadow = 0.0;
		
		result = Shading( pos, norm, shadow );
		
		// fog
		result = mix ( vec3(.7,.9,1.2), result, exp(-t*t*.0002) );
	}
	

	gl_FragColor = vec4( result, 1.0 );
}
