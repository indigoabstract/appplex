// https://www.shadertoy.com/view/XsfGR7
/* 
If you think this shader is an useful example, 
please like it :-)

reference:
https://www.shadertoy.com/view/Mdl3Rr

change log:
- 2013-04-10 initial release
*/

#define LIGHT_ANIMATION
//#define LOW_QUALITY		// uncomment this if this shader runs too slow on your PC
//#define ULTRA_QUALITY 	// uncomment this if you have a really fast GPU :-)
#define SMOKE				// comment this if you think the smoke effect is too annoying

float gAnimTime = iGlobalTime * 0.5;

struct Ray
{
	vec3 org;
	vec3 dir;
};
	
float hash (float n)
{
	return fract(sin(n)*43758.5453);
}

float noise (in vec3 x)
{
	vec3 p = floor(x);
	vec3 f = fract(x);

	f = f*f*(3.0-2.0*f);

	float n = p.x + p.y*57.0 + 113.0*p.z;

	float res = mix(mix(mix( hash(n+  0.0), hash(n+  1.0),f.x),
						mix( hash(n+ 57.0), hash(n+ 58.0),f.x),f.y),
					mix(mix( hash(n+113.0), hash(n+114.0),f.x),
						mix( hash(n+170.0), hash(n+171.0),f.x),f.y),f.z);
	return res;
}

float udRoundBox( in vec3 p, in vec3 b, in float r )
{
	return length(max(abs(p)-b,0.0))-r;
}

float sdPlane( in vec3 p, in vec4 n )
{
	// n must be normalized
	return dot( p, n.xyz ) + n.w;
}

float opU( in float d1, in float d2 )
{
	return min(d1,d2);
}

vec3 translate( in vec3 v, in vec3 t )
{
	return v - t;
}

float scene( in vec3 pos )
{
	vec4 plane = vec4( 0.0, 1.0, 0.0, 0.0 ); // xyz, d
	
	vec4 boxd1 = vec4( 0.5, 3.5, 0.5, 0.25 ); // sxyz, r
	vec4 boxp1 = vec4( 0.0, 3.5, 0.0, 0.0 ); // xyz, 0
	boxd1.xyz -= boxd1.w;
	
	vec4 boxd2 = vec4( 3.0, 0.5, 0.5, 0.25 ); // sxyz, r
	vec4 boxp2 = vec4( 0.0, 4.5, 0.0, 0.0 ); // xyz, 0
	boxd2.xyz -= boxd2.w;
	
	
	float d = 99999.0;
	
	d = opU( d, udRoundBox( translate( pos, boxp1.xyz ), boxd1.xyz, boxd1.w ) );
	d = opU( d, udRoundBox( translate( pos, boxp2.xyz ), boxd2.xyz, boxd2.w ) );
	d = opU( d, sdPlane( pos, plane ) );
	
	return d;
}

vec3 sceneNormal( in vec3 pos )
{
    vec3 eps = vec3( 0.001, 0.0, 0.0 );
	vec3 nor;
	nor.x = scene( pos + eps.xyy ) - scene( pos - eps.xyy );
	nor.y = scene( pos + eps.yxy ) - scene( pos - eps.yxy );
	nor.z = scene( pos + eps.yyx ) - scene( pos - eps.yyx );
	return normalize( nor );
}

bool raymarch( in Ray ray, in int maxSteps, out vec3 hitPos, out vec3 hitNrm )
{
	const float hitThreshold = 0.0001;

	bool hit = false;
	hitPos = ray.org;

	vec3 pos = ray.org;

	for ( int i = 0; i < 256; i++ )
	{
		if ( i >= maxSteps )
			break;
		float d = scene( pos );

		if ( d < hitThreshold )
		{
			hit = true;
			hitPos = pos;
			hitNrm = sceneNormal( pos );
			break;
		}
		pos += d * ray.dir;
	}
	return hit;
}

#ifdef LOW_QUALITY
#define INSCATTER_STEPS 24
#else
#	ifdef ULTRA_QUALITY
#define INSCATTER_STEPS 64
#	else
#define INSCATTER_STEPS 48
#	endif
#endif


float raySphereIntersect( in vec3 ro, in vec3 rd, in vec4 sph )
{
	vec3 oc = ro - sph.xyz; // looks like we are going place sphere from an offset from ray origin, which is = camera
	float b = 2.0 * dot( oc, rd );
	float c = dot( oc, oc ) - sph.w * sph.w; // w should be size
	float h = b * b - 4.0 * c;
	if ( h < 0.0 )
	{
		return -10000.0;
	}
	float t = (-b - sqrt(h)) / 2.0;
	
	return t;
}

vec3 inscatter( in Ray rayEye, in vec4 light, in vec3 screenPos, in float sceneTraceDepth )
{
	vec3 rayEeyeNDir = normalize( rayEye.dir );
	
	// the eye ray does not intersect with the light, so skip computing
	if ( raySphereIntersect( rayEye.org, rayEeyeNDir, light ) < -9999.0 )
		return vec3( 0.0 );
	
	float scatter = 0.0;
	float invStepSize = 1.0 / float( INSCATTER_STEPS );
	
	vec3 hitPos, hitNrm;
	vec3 p = rayEye.org;
	vec3 dp = rayEeyeNDir * invStepSize * sceneTraceDepth;
	
	// apply random offset to minimize banding artifacts.
	p += dp * noise( screenPos ) * 1.5;
	
	for ( int i = 0; i < INSCATTER_STEPS; ++i )
	{
		p += dp;
		
		Ray rayLgt;
		rayLgt.org = p;
		rayLgt.dir = light.xyz - p;
		float dist2Lgt = length( rayLgt.dir );
		rayLgt.dir /= 8.0;
		
		float sum = 0.0;
		if ( !raymarch( rayLgt, 16, hitPos, hitNrm ) )
		{
			// a simple falloff function base on distance to light
			float falloff = 1.0 - pow( clamp( dist2Lgt / light.w, 0.0, 1.0 ), 0.125 );
			sum += falloff;
			
#ifdef SMOKE
			float smoke = noise( 1.25 * ( p + vec3( gAnimTime, 0.0, 0.0 ) ) ) * 0.375;
			sum += smoke * falloff;
#endif
		}
		
		scatter += sum;
	}
	
	scatter *= invStepSize; // normalize the scattering value
	scatter *= 8.0; // make it brighter
	
	return vec3( scatter );
}

float softshadow( in Ray ray, in float mint, in float maxt, in float k )
{
	float t = mint;
	float res = 1.0;
    for ( int i = 0; i < 128; ++i )
    {
        float h = scene( ray.org + ray.dir * t );
        if ( h < 0.001 )
            return 0.0;
		
		res = min( res, k * h / t );
        t += h;
		
		if ( t > maxt )
			break;
    }
    return res;
}

void main(void)
{
	vec2 ndcXY = -1.0 + 2.0 * gl_FragCoord.xy / iResolution.xy;
	float aspectRatio = iResolution.x / iResolution.y;
	
	// camera XYZ in world space
	vec3 camWsXYZ = vec3( 0.0, 3.0, 5.0 );
	
	// construct the ray in world space
	Ray ray;
	ray.org = camWsXYZ;
	ray.dir = vec3( ndcXY * vec2( aspectRatio, 1.0 ), -1.0 ); // OpenGL is right handed
	
	// define the point light in world space (XYZ, range)
	vec4 lightWs = vec4( 0.0, 4.5, -4.0, 10.0 );
#ifdef LIGHT_ANIMATION
	lightWs.x += sin( gAnimTime ) * 2.0;
	lightWs.y += cos( gAnimTime ) * 2.0;
#endif
	
	vec3 sceneWsPos;
	vec3 sceneWsNrm;
	
	vec4 c = vec4( 0.0 );
	
	if ( raymarch( ray, 128, sceneWsPos, sceneWsNrm ) )
	{
		// apply simple depth fog
		float viewZ = sceneWsPos.z - camWsXYZ.z;
		float fog = clamp( ( viewZ + 20.0 ) / 5.0 , 0.0, 1.0 );
		fog = fog * fog;
		c.rgb = vec3( 0.125 * fog );
	}
	
	// apply scattering of the
	c.rgb += inscatter( ray, lightWs, vec3( gl_FragCoord.xy, 0.0 ), 12.0 );
	
	// color correction - Sherlock color palette
	c.r = smoothstep( 0.0, 1.0, c.r );
	c.g = smoothstep( 0.0, 1.0, c.g - 0.1 );
	c.b = smoothstep(-0.3, 1.3, c.b );
	
	gl_FragColor = c;
}
