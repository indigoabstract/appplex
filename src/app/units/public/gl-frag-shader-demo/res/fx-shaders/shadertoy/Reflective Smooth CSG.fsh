// https://www.shadertoy.com/view/MdX3Rl

// iChannel0: t14
// iChannel1: c1

// Created by Ben Weston - 2013
// License Creative Commons Attribution-NonCommercial-ShareAlike 3.0 Unported License.

// constants
const float tau = 6.28318530717958647692;
float Noise( in vec3 x );



// ---USER TWEAKABLE THINGS!---

const float epsilon = .003;
const float normalPrecision = .1;
const float shadowOffset = .1;
const int traceDepth = 100; // takes time
const float drawDistance = 100.0;

const vec3 CamPos = vec3(0,40.0,-40.0);
const vec3 CamLook = vec3(0,0,0);

const vec3 lightDir = vec3(.7,1,-.1);
const vec3 fillLightDir = vec3(0,0,-1);
const vec3 lightColour = vec3(1.1,1.05,1);
const vec3 fillLightColour = vec3(.38,.4,.42);
	
// This should return continuous positive values when outside and negative values inside,
// which roughly indicate the distance of the nearest surface.
float Isosurface( vec3 ipos )
{
	// animate the object rotating
	float ang = iGlobalTime*tau/25.0;
	float ang2 = iGlobalTime*tau/125.0;
	float s = sin(ang), c = cos(ang);
	float s2 = sin(ang2), c2 = cos(ang2);
	vec3 pos;
	pos.y = c*ipos.y-s*ipos.z;
	pos.z = c*ipos.z+s*ipos.y;
	pos.x = ipos.x*c2+pos.z*s2;
	pos.z = pos.z*c2-ipos.x*s2;


	// smooth csg
	const float smoothing = .25;

	return
		log(
			// intersection
			1.0/(
				// union
				1.0/(
					// intersection
					exp((length(pos.xz)-10.0)/smoothing) +
					exp((-(length(pos.xz)-7.0))/smoothing) +
					exp((-(length(vec2(8.0,0)+pos.zy)-5.0))/smoothing) +
					exp((pos.y-10.0)/smoothing) +
					exp((-pos.y-10.0)/smoothing)
					)
				+ exp(-(length(pos+15.0*vec3(sin(iGlobalTime*.07),sin(iGlobalTime*.13),sin(iGlobalTime*.1)))-5.0))
				)
			// trim it with a plane
			//+ exp((dot(pos,normalize(vec3(-1,-1,1)))-10.0-10.0*sin(iGlobalTime*.17))/smoothing)
		)*smoothing
		+ Noise(pos*16.0)*.08/16.0; // add some subtle texture
}


// alpha controls reflection
vec4 Shading( vec3 pos, vec3 norm, float shadow, vec3 rd )
{
	vec3 albedo = vec3(1);//mix( vec3(1,.8,.7), vec3(.5,.2,.1), Noise(pos*vec3(1,10,1)) );

	vec3 l = shadow*lightColour*max(0.0,dot(norm,normalize(lightDir)));
	vec3 fl = fillLightColour*(dot(norm,normalize(fillLightDir))*.5+.5);
	
	vec3 view = normalize(-rd);
	vec3 h = normalize(view+lightDir);
	float specular = pow(max(0.0,dot(h,norm)),2000.0);
	
	float fresnel = pow( 1.0 - dot( view, norm ), 5.0 );
	fresnel = mix( .7, 1.0, min(1.0,fresnel) );
	
	return vec4( albedo*(l+fl)*(1.0-fresnel) + shadow*specular*32.0*lightColour, fresnel );
}

const vec3 FogColour = vec3(.1,.2,.5);

vec3 SkyColour( vec3 rd )
{
	// hide cracks in cube map
	rd -= sign(abs(rd.xyz)-abs(rd.yzx))*.01;

	//return mix( vec3(.2,.6,1), FogColour, abs(rd.y) );
	vec3 ldr = textureCube( iChannel1, rd ).rgb;
	
	// fake hdr
	vec3 hdr = 1.0/(1.2-ldr) - 1.0/1.2;
	
	return hdr;
}

// ---END OF USER TWEAKABLE THINGS!---


// backend code, hopefully needn't be edited:

float Noise( in vec3 x )
{
    vec3 p = floor(x.xzy);
    vec3 f = fract(x.xzy);
//	f = f*f*(3.0-2.0*f);
	vec3 f2 = f*f; f = f*f2*(10.0-15.0*f+6.0*f2);
	
//cracks cause a an artefact in normal, of course
	
	// there's an artefact because the y channel almost, but not exactly, matches the r channel shifted (37,17)
	// this artefact doesn't seem to show up in chrome, so I suspect firefox uses different texture compression.
	vec2 uv = (p.xy+vec2(37.0,17.0)*p.z) + f.xy;
	vec2 rg = texture2D( iChannel0, (uv+0.5)/256.0, -100.0 ).ba;
	return mix( rg.y, rg.x, f.z )-.5;
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
	
	// reduce edge sparkles, caused by reflections on failed positions
	if ( dist > epsilon )
		return drawDistance+1.0;
	
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


void Humbug( inout vec4 result, inout vec3 ro, inout vec3 rd )
{
	if ( result.a < .01 )
		return; // continue; // break;
	
	float t = Trace(ro,rd);
	
	vec4 sample = vec4( SkyColour( rd ), 0 );
	
	vec3 norm;
	if ( t < drawDistance )
	{
		ro = ro+t*rd;
		
		norm = GetNormal(ro);
		
		// shadow test
		float shadow = 1.0;
		if ( Trace( ro+lightDir*shadowOffset, lightDir ) < drawDistance )
			shadow = 0.0;
		
		sample = Shading( ro, norm, shadow, rd );
	}
	
	result.rgb += sample.rgb*result.a;
	result.a *= sample.a;
	result.a = clamp(result.a,0.0,1.0); // without this, chrome shows black!
	
	//		// fog
	//		result = mix ( vec4(FogColour, 0), result, exp(-t*t*.0002) );
	
	rd = reflect(rd,norm);
	
	ro += rd*shadowOffset;
}


void main(void)
{
	vec2 uv = gl_FragCoord.xy / iResolution.xy;

	vec3 camPos = CamPos;
	vec3 camLook = CamLook;

	vec2 camRot = .5*tau*(iMouse.xy-iResolution.xy*.5)/iResolution.x;
	camPos.yz = cos(camRot.y)*camPos.yz + sin(camRot.y)*camPos.zy*vec2(1,-1);
	camPos.xz = cos(camRot.x)*camPos.xz + sin(camRot.x)*camPos.zx*vec2(1,-1);
	
	if ( Isosurface(camPos) <= 0.0 )
	{
		// camera inside ground
		gl_FragColor = vec4(0,0,0,0);
		return;
	}

	vec3 ro = camPos;
	vec3 rd;
	rd = GetRay( camLook-camPos, 2.0, uv );
	rd = normalize(rd);
	
	vec4 result = vec4(0,0,0,1);

	for ( int bounce = 0; bounce < 3; bounce ++ )
		Humbug( result, ro, rd );
	
	gl_FragColor = result;
}
